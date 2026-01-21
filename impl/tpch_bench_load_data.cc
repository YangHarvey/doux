#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <random>
#include <ctime>
#include <filesystem>
#include <time.h>
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "../mod/util.h"
#include "../mod/stats.h"
#include "../mod/cxxopts.hpp"
#include "../db/version_set.h"
#include "tpch_base.h"

using namespace leveldb;
using namespace adgMod;
using std::string;
using std::cout;
using std::endl;
using std::to_string;
using std::vector;
using std::map;
using std::ifstream;
using std::string;

void EncodeRow(const vector<string>& rows, string* pkey, string* vkey, string *secondary_key, string* value, size_t value_size = 314) {
    // encode primary key
    {
        pkey->append(fill_key(rows[l_orderkey], 16));
        pkey->append(fill_key(rows[l_linenumber], 8));
    }

    // encode value key
    {
        double shipdate = uniformDate(rows[l_shipdate].c_str());
        double quantity = std::stod(rows[l_quantity].c_str());
        vkey->append(reinterpret_cast<const char*>(&shipdate), sizeof(double));
        vkey->append(reinterpret_cast<const char*>(&quantity), sizeof(double));
    }

    // encode secondary key
    {
        secondary_key->append(fill_value("index", 8));

        double shipdate = uniformDate(rows[l_shipdate].c_str());
        double quantity = std::stod(rows[l_quantity].c_str());
        double receiptdate = uniformDate(rows[l_receiptdate].c_str());

        secondary_key->append(reinterpret_cast<const char*>(&shipdate), sizeof(double));
        secondary_key->append(reinterpret_cast<const char*>(&quantity), sizeof(double));
        secondary_key->append(reinterpret_cast<const char*>(&receiptdate), sizeof(double));
        secondary_key->append(fill_key(rows[l_orderkey], 16));
        secondary_key->append(fill_key(rows[l_linenumber], 8));
    }

    // encode value
    {
        double shipdate = uniformDate(rows[l_shipdate].c_str());
        double quantity = std::stod(rows[l_quantity].c_str());
        if(adgMod::MOD == 10 || adgMod::MOD == 13) {
            // doux
            uint32_t val1 = shipdate * 100;
            uint32_t val2 = quantity;
            auto sort_key = doux::MortonCode<2, 32>::Encode({val1, val2});
            uint64_t sk = static_cast<uint64_t>(sort_key);

            char buf[sizeof(uint64_t)];
            EncodeFixed64(buf, sk);
            value->append(buf, sizeof(uint64_t));
        } else {
            value->append(reinterpret_cast<const char*>(&shipdate), sizeof(double));
            value->append(reinterpret_cast<const char*>(&quantity), sizeof(double));
        }

        uint64_t partkey = std::stoull(rows[l_partkey].c_str(), nullptr, 10);
        value->append(reinterpret_cast<const char*>(&partkey), sizeof(uint64_t));

        uint64_t suppkey = std::stoull(rows[l_suppkey].c_str(), nullptr, 10);
        value->append(reinterpret_cast<const char*>(&suppkey), sizeof(uint64_t));

        double extendedprice = std::atof(rows[l_extendedprice].c_str());
        value->append(reinterpret_cast<const char*>(&extendedprice), sizeof(double));
        
        double discount = std::atof(rows[l_discount].c_str());
        value->append(reinterpret_cast<const char*>(&discount), sizeof(double));

        double tax = std::atof(rows[l_tax].c_str());
        value->append(reinterpret_cast<const char*>(&tax), sizeof(double));

        char returnflag = rows[l_returnflag][0];
        value->append(1, returnflag);  // 固定1字节

        char linestatus = rows[l_linestatus][0];
        value->append(1, linestatus);  // 固定1字节

        double commitdate = uniformDate(rows[l_commitdate].c_str());
        value->append(reinterpret_cast<const char*>(&commitdate), sizeof(double));

        if(value_size == 314) {
            // default value size
            string shipinstruct = fill_value(rows[l_shipinstruct], 64);
            value->append(shipinstruct);  // 固定64字节

            string shipmode = fill_value(rows[l_shipmode], 64);
            value->append(shipmode);  // 固定64字节

            string comment = fill_value(rows[l_comment], 128);
            value->append(comment);  // 固定128字节
        } else if(value_size == 64) {
            // value size == 64, 58 + 2 + 2 + 2
            string shipinstruct = std::string(rows[l_shipinstruct], 2);
            value->append(shipinstruct);  // 固定2字节

            string shipmode = std::string(rows[l_shipmode], 2);
            value->append(shipmode);  // 固定2字节

            string comment = std::string(rows[l_comment], 2);
            value->append(comment);  // 固定2字节
        } else if(value_size == 256) {
            string shipinstruct = fill_value(rows[l_shipinstruct], 66);
            value->append(shipinstruct);  // 固定66字节

            string shipmode = fill_value(rows[l_shipmode], 66);
            value->append(shipmode);  // 固定66字节

            string comment = fill_value(rows[l_comment], 66);
            value->append(comment);  // 固定66字节
        } else if(value_size == 1024) {
            string shipinstruct = fill_value(rows[l_shipinstruct], 322);
            value->append(shipinstruct);  // 固定322字节

            string shipmode = fill_value(rows[l_shipmode], 322);
            value->append(shipmode);  // 固定322字节

            string comment = fill_value(rows[l_comment], 322);
            value->append(comment);  // 固定322字节
        } else {
            throw std::runtime_error("Invalid value size");
        }
    }
}

int main(int argc, char *argv[]) {
    std::string input_filename, db_location;
    size_t data_count;

    cxxopts::Options commandline_options("leveldb tpch test", "Testing leveldb based on tpch lineitem.");
    commandline_options.add_options()
            ("f,input_file", "the filename of input file", cxxopts::value<string>(input_filename)->default_value(""))
            ("c,count", "the number of data to load", cxxopts::value<size_t>(data_count)->default_value("1000000"))
            ("m,modification", "if set, run our modified version", cxxopts::value<int>(adgMod::MOD)->default_value("0"))
            ("d,directory", "the directory of db", cxxopts::value<string>(db_location)->default_value("/mnt/doux/testdb"))
            ("si", "use secondary index", cxxopts::value<bool>(adgMod::use_secondary_index)->default_value("false"));

    auto result = commandline_options.parse(argc, argv);
    
    adgMod::key_size = 16;
    adgMod::value_size = 314;

    DB* db;
    Options options;
    ReadOptions& read_options = adgMod::read_options;
    WriteOptions& write_options = adgMod::write_options;
    Status status;

    options.create_if_missing = true;
    write_options.sync = false;

    adgMod::Stats* instance = adgMod::Stats::GetInstance();
    vector<vector<size_t>> times(30);
    instance->ResetAll();

    cout << "input_filename: " << input_filename << endl;
    cout << "modification: " << adgMod::MOD << endl;
    cout << "db_location: " << db_location << endl;
    cout << "data count: " << data_count << endl;

    // db_location不存在则创建
    if(!std::filesystem::exists(db_location)) {
        std::filesystem::create_directories(db_location);
    }
    status = DB::Open(options, db_location, &db);
    assert(status.ok() && "Open Error");

    std::ifstream input(input_filename);
    string src;
    vector<string> row;

    if (adgMod::MOD == 0) {
        // leveldb
        for(size_t i = 0; i < data_count; i++) {
            getline(input, src);
            splitRow(src, row);
            string pkey, vkey, secondary_key, value;

            EncodeRow(row, &pkey, &vkey, &secondary_key, &value);

            instance->StartTimer(9);
            if(!adgMod::use_secondary_index) {
                // no secondary index
                status = db->Put(write_options, pkey, value);
            } else {
                // if use secondary index
                status = db->sPut(write_options, pkey, secondary_key, value);
            }
            assert(status.ok() && "File Put Error");
            instance->PauseTimer(9, true);

            row.clear();
        }
    } else if (adgMod::MOD == 10) {
        // doux
        for(size_t i = 0; i < data_count; i++) {
            getline(input, src);
            splitRow(src, row);
            string pkey, vkey, secondary_key, value;
            EncodeRow(row, &pkey, &vkey, &secondary_key, &value);

            instance->StartTimer(9);
            status = db->Put(write_options, pkey, value);
            assert(status.ok() && "File Put Error");
            instance->PauseTimer(9, true);

            row.clear();
        }
    }


    cout << "Put Complete" << endl;
    if(adgMod::MOD == 12) {
        db->runAllColocationGC();
    }
    adgMod::db->WaitForBackground();
    delete db;

    input.close();

    std::cout << "Time taken: " << instance->ReportTime(9) << " seconds" << std::endl;
    return 0;
}