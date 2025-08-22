#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <random>
#include <ctime>
#include <time.h>
#include "db/builder.h"
#include "../tpch_base.h"
#include "db/builder.h"
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "../mod/util.h"
#include "../mod/stats.h"
#include "../mod/cxxopts.hpp"
#include "../db/version_set.h"

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

const int MAX_LINEITEM_COUNT = 10000;


/* TPCH lineitem
 * l_orderkey       uint64_t(8)    pk1
 * l_partkey        uint64_t(8)    
 * l_suppkey        uint64_t(8)    
 * l_linenumber     uint64_t(8)    pk2
 * l_quantity       uint32_t(4)    
 * l_extendedprice  double(8)
 * l_discount       double(8)
 * l_tax            double(8)
 * l_returnflag     char(1)
 * l_linestatus     char(1)
 * l_shipdate       uint32_t(4)
 * l_commitdate     uint32_t(4)
 * l_receiptdate    uint32_t(4)
 * l_shipinstruct   char(64)
 * l_shipmode       char(64)
 * l_comment        char(128)       
 */

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distribution(1, 999);

void EncodeRow(const vector<string>& row, string* key, string* value, int value_size) {
    // Encode key
    key->append(fill_key(row[l_orderkey], 16));
    key->append(fill_key(row[l_linenumber], 8));

    // Encode values

    // [shipdate, quantity, l_receiptdate]作为前三个属性
    uint32_t shipdate = 0;
    shipdate = uniformDate(row[l_shipdate].c_str());
    PutBigEndianFixed32(value, shipdate);

    uint32_t quantity = 0;
    quantity = std::stoul(row[l_quantity].c_str(), nullptr, 10);
    PutBigEndianFixed32(value, quantity);

    uint32_t receiptdate = 0;
    receiptdate = uniformDate(row[l_receiptdate].c_str());
    PutBigEndianFixed32(value, receiptdate);

    // 
    uint64_t partkey = 0;
    partkey = std::stoull(row[l_partkey].c_str(), nullptr, 10);
    PutFixed64(value, partkey);

    uint64_t suppkey = 0;
    suppkey = std::stoull(row[l_suppkey].c_str(), nullptr, 10);
    PutFixed64(value, suppkey);

    double extendedprice = 0;
    extendedprice = std::atof(row[l_extendedprice].c_str());
    PutFixed64(value, extendedprice);

    double discount = 0;
    discount = std::atof(row[l_discount].c_str());
    PutFixed64(value, discount * 100);

    double tax = 0;
    tax = std::atof(row[l_tax].c_str());
    PutFixed64(value, tax);

    // 使用固定长度编码替代变长编码
    char returnflag = ' ';
    returnflag = row[l_returnflag][0];
    value->append(1, returnflag);  // 固定1字节

    char linestatus = ' ';
    linestatus = row[l_linestatus][0];
    value->append(1, linestatus);  // 固定1字节

    uint32_t commitdate = 0;
    commitdate = uniformDate(row[l_commitdate].c_str());
    PutFixed32(value, commitdate);

    if(value_size == 314) {
        // default value size
        string shipinstruct = fill_value(row[l_shipinstruct], 64);
        value->append(shipinstruct);  // 固定64字节

        string shipmode = fill_value(row[l_shipmode], 64);
        value->append(shipmode);  // 固定64字节

        string comment = fill_value(row[l_comment], 128);
        value->append(comment);  // 固定128字节
    } else if(value_size == 64) {
        // value size == 64, 58 + 2 + 2 + 2
        string shipinstruct = std::string(row[l_shipinstruct], 2);
        value->append(shipinstruct);  // 固定2字节

        string shipmode = std::string(row[l_shipmode], 2);
        value->append(shipmode);  // 固定2字节

        string comment = std::string(row[l_comment], 2);
        value->append(comment);  // 固定2字节
    } else if(value_size == 256) {
        string shipinstruct = fill_value(row[l_shipinstruct], 66);
        value->append(shipinstruct);  // 固定66字节

        string shipmode = fill_value(row[l_shipmode], 66);
        value->append(shipmode);  // 固定66字节

        string comment = fill_value(row[l_comment], 66);
        value->append(comment);  // 固定66字节
    } else if(value_size == 1024) {
        string shipinstruct = fill_value(row[l_shipinstruct], 322);
        value->append(shipinstruct);  // 固定322字节

        string shipmode = fill_value(row[l_shipmode], 322);
        value->append(shipmode);  // 固定322字节

        string comment = fill_value(row[l_comment], 322);
        value->append(comment);  // 固定322字节
    } else {
        throw std::runtime_error("Invalid value size");
    }
}

// build secondary index
// shipdate + quantity + receiptdate + pk as secondary index
void encodeSecondaryKey(const vector<string>& row, string* secondary_key) {
    // encode index
    std::string index_key = fill_value("index", 8);
    secondary_key->append(index_key);

    uint32_t shipdate = 0;
    shipdate = uniformDate(row[l_shipdate].c_str());
    PutBigEndianFixed32(secondary_key, shipdate);

    uint32_t quantity = 0;
    quantity = std::stoul(row[l_quantity].c_str(), nullptr, 10);
    PutBigEndianFixed32(secondary_key, quantity);

    uint32_t receiptdate = 0;
    receiptdate = uniformDate(row[l_receiptdate].c_str());
    PutBigEndianFixed32(secondary_key, receiptdate);

    // Encode key
    secondary_key->append(fill_key(row[l_orderkey], 16));
    secondary_key->append(fill_key(row[l_linenumber], 8));
}

/*
    Query Type
    select
        sum(l_extendedprice * l_discount) as revenue
    from
            lineitem
    where
            l_shipdate >= date '1993-01-01'
            and l_shipdate < date '1994-01-01'
            and l_discount between 0.03 and 0.05
            and l_quantity < 24;
*/

void delete_db(string db_location) {
    string command = "rm -rf " + db_location;
    system(command.c_str());
    system("sync; echo 3 | tee /proc/sys/vm/drop_caches");
    cout << "delete and trim complete" << endl;
}

int main(int argc, char *argv[]) {
    int num_iteration;
    string input_filename, db_location;
    bool print_file_info, evict, unlimit_fd;
    uint32_t start1, end1, start2, end2;

    int query_type;
    bool predefined_range;

    adgMod::key_size = 16;
    adgMod::value_size = 314;

    bool run_query = false;  // 是否运行查询

    cxxopts::Options commandline_options("leveldb tpch test", "Testing leveldb based on tpch lineitem.");
    commandline_options.add_options()
            ("f,input_file", "the filename of input file", cxxopts::value<string>(input_filename)->default_value(""))
            ("m,modification", "if set, run our modified version", cxxopts::value<int>(adgMod::MOD)->default_value("0"))
            ("i,iteration", "the number of iterations of a same size", cxxopts::value<int>(num_iteration)->default_value("0"))
            ("h,help", "print help message", cxxopts::value<bool>()->default_value("false"))
            ("d,directory", "the directory of db", cxxopts::value<string>(db_location)->default_value("/mnt/doux/testdb"))
            ("init_db", "init database", cxxopts::value<bool>(fresh_write)->default_value("false"))
            ("c,uncache", "evict cache", cxxopts::value<bool>(evict)->default_value("true"))
            ("file_info", "print the file structure info", cxxopts::value<bool>(print_file_info)->default_value("true"))
            ("unlimit_fd", "unlimit fd", cxxopts::value<bool>(unlimit_fd)->default_value("false"))
            ("dummy", "dummy option")
            ("run_query", "run query", cxxopts::value<bool>(run_query)->default_value("false"))
            ("predefined_range", "use predefined range", cxxopts::value<bool>(predefined_range)->default_value("false"))
            ("filter", "use filter", cxxopts::value<bool>(adgMod::use_filter)->default_value("false"))
            ("start1", "start for attribute 1", cxxopts::value<uint32_t>(start1)->default_value("1"))
            ("end1", "end for attribute 1", cxxopts::value<uint32_t>(end1)->default_value("10"))
            ("start2", "start for attribute 2", cxxopts::value<uint32_t>(start2)->default_value("1"))
            ("end2", "end for attribute 2", cxxopts::value<uint32_t>(end2)->default_value("10"))
            ("v, value_size", "value size", cxxopts::value<int>(adgMod::value_size)->default_value("314"));

    auto result = commandline_options.parse(argc, argv);
    if (result.count("help")) {
        printf("%s", commandline_options.help().c_str());
        exit(0);
    }
    
    adgMod::fd_limit = unlimit_fd ? 1024 * 1024 : 1024;
    DB* db;
    Options options;
    ReadOptions& read_options = adgMod::read_options;
    WriteOptions& write_options = adgMod::write_options;
    Status status;
    
    // 设置为1MB，记录什么时候报错
    options.max_file_size = 4 * 1024 * 1024;

    options.create_if_missing = true;
    write_options.sync = false;
    read_options.start1 = start1;
    read_options.end1 = end1;
    read_options.start2 = start2;
    read_options.end2 = end2;
    read_options.start = doux::MortonCode<2, 32>::Encode({start1, start2}).data_;
    read_options.end = doux::MortonCode<2, 32>::Encode({end1, end2}).data_;
    doux::AABB<2, 32> aabb = {read_options.start, read_options.end};
    doux::Region<2, 32> region = aabb.ToIntervals();

    adgMod::Stats* instance = adgMod::Stats::GetInstance();
    vector<vector<size_t>> times(20);
    instance->ResetAll();

    int line_count = 0;
    if (!input_filename.empty() && fresh_write) {
        // delete the db and open the db
        delete_db(db_location);
        status = DB::Open(options, db_location, &db);
        assert(status.ok() && "Open Error");

        // load the data
        std::string last_key, last_value;
        ifstream input(input_filename);
        string src;
        vector<string> row;
        while (getline(input, src) && line_count < MAX_LINEITEM_COUNT) {
            splitRow(src, row);
            string key, value;
            EncodeRow(row, &key, &value, adgMod::value_size);

            status = db->Put(write_options, key, value);
            last_key = key;
            last_value = value;
            assert(status.ok() && "File Put Error");

            row.clear();
            line_count++;
        }
        std::cout << "last_key: " << last_key << ", last_value: " << last_value << std::endl;

        cout << "Put Complete" << endl;
        
        if (print_file_info) {
            db->PrintFileInfo();
        }
        if (adgMod::db != nullptr) {
            adgMod::db->WaitForBackground();
        }
        delete db;
    }

    // 跑一次查询
    if(run_query) { 
        if (evict) {
            system("sync; echo 3 | tee /proc/sys/vm/drop_caches");
            cout << "Drop all caches" << endl;
        }
    
        cout << "--------------------------------Begin Test TPC-H Q6!--------------------------------\n";
    
        status = DB::Open(options, db_location, &db);
        assert(status.ok() && "Open Error");
        if (adgMod::db != nullptr) {
            adgMod::db->WaitForBackground();
        }
    
        Iterator* db_iter;
        uint64_t res_count = 0;
        // Doux: Run Query 6

        instance->StartTimer(17);

        uint64_t scan_len = 0;
        cout << "Morton code from " << read_options.start << " to "  << read_options.end << endl;
        for (const auto interval : region.intervals_) {
            scan_len += interval.end_ - interval.start_;
            cout << "interval from " << interval.start_ << " to " << interval.end_ << endl;
        }
        cout << "scan length: " << scan_len << endl;

        for(const auto interval : region.intervals_) {
            read_options.start = interval.start_;
            read_options.end = interval.end_;
            db_iter = db->NewVIterator(read_options);
            instance->StartTimer(4);
            std::cout << "seek to " << interval.start_ << std::endl;
            std::cout << "seek to " << reinterpret_cast<const char*>(&interval.start_) << std::endl;
            db_iter->Seek(Slice(reinterpret_cast<const char*>(&interval.start_), sizeof(uint64_t)));
            if(db_iter->Valid()) {
                std::cout << "actually seek to " << db_iter->key().ToString() << std::endl;
            } else {
                std::cout << "seek to " << interval.start_ << " failed" << std::endl;
            }
            instance->PauseTimer(4);

            instance->StartTimer(17);
            for (; db_iter->Valid() && VKSliceCompare(db_iter->key(), Slice(reinterpret_cast<const char*>(&interval.end_), sizeof(uint64_t))) == false; db_iter->Next()) {
                Slice key = db_iter->key();
                Slice value = db_iter->value();
                ++res_count;
            }
            instance->PauseTimer(17);
        }
        std::cout << "res_count: " << res_count << std::endl;
        instance->PauseTimer(17);

        // Report Time
        instance->ReportTime();
        for (int s = 0; s < times.size(); ++s) {
            times[s].push_back(instance->ReportTime(s));
        }
        if (adgMod::db != nullptr) {
            adgMod::db->WaitForBackground();
        }
        sleep(10);
    
        cout << "--------------------------------End Test TPC-H Q6!--------------------------------\n";
    }

    return 0;
}