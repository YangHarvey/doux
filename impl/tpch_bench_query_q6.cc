#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <random>
#include <ctime>
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

double deserialize_val(const Slice& value, int attr) {
    const double* ptr = reinterpret_cast<const double*>(value.data());
    switch(attr) {
        case l_shipdate:
            return ptr[0];
        case l_quantity:
            return ptr[1];
    }
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

int main(int argc, char *argv[]) {
    std::string input_filename, db_location;
    int query_size;

    cxxopts::Options commandline_options("leveldb tpch test", "Testing leveldb based on tpch lineitem.");
    commandline_options.add_options()
            ("f,input_file", "the filename of input file", cxxopts::value<string>(input_filename)->default_value(""))
            ("m,modification", "if set, run our modified version", cxxopts::value<int>(adgMod::MOD)->default_value("0"))
            ("q,query_size", "the number of queries", cxxopts::value<int>(query_size)->default_value("5"))
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

    status = DB::Open(options, db_location, &db);
    assert(status.ok() && "Open Error");

    Iterator* db_iter;

    int op, id;
    double low[2], high[2];
    std::ifstream queryFile(input_filename);
    for(int query_idx = 0; query_idx < query_size; ++query_idx) {
        queryFile >> op >> id >> low[0] >> low[1] >> high[0] >> high[1];
        std::cout << "query: " << low[0] << " " << high[0] << " " << low[1] << " " << high[1] << std::endl;

        instance->StartTimer(13);
        if(adgMod::MOD == 0 && !adgMod::use_secondary_index) {
            // LevelDB
            db_iter = db->NewIterator(read_options);

            uint64_t res_count = 0;
            db_iter->SeekToFirst();
            for (; db_iter->Valid(); db_iter->Next()) {
                Slice primary_key = db_iter->key();
                Slice value = db_iter->value();

                // 检查是否在范围内
                double shipdate = deserialize_val(value, l_shipdate);
                double quantity = deserialize_val(value, l_quantity);

                if(shipdate >= low[0] && shipdate <= high[0] && quantity >= low[1] && quantity <= high[1]) {
                    ++res_count;
                }
            }

            cout << "Num of entries within range: " << res_count << endl;

        } else {
            std::cerr << "MOD is not supported" << std::endl;
        }

        instance->PauseTimer(13, true);
        delete db_iter;
    }

    std::cout << "Total time: " << instance->ReportTime(13) << " ns" << std::endl;
    std::cout << "Average time: " << instance->ReportTime(13) / query_size << " ns" << std::endl;
    std::cout << "Average time per entry: " << instance->ReportTime(13) / query_size / 1e9 << " s" << std::endl;

    delete db;

    return 0;
}