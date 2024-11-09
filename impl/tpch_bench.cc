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

enum Lineitem {
    l_orderkey = 0,
    l_partkey = 1,
    l_suppkey = 2,
    l_linenumber = 3,
    l_quantity = 4,
    l_extendedprice = 5,
    l_discount = 6,
    l_tax = 7,
    l_returnflag = 8,   
    l_linestatus = 9,   
    l_shipdate = 10,    
    l_commitdate = 11,   
    l_receiptdate = 12,
    l_shipinstruct = 13, 
    l_shipmode = 14,     
    l_comment = 15     
};


void splitRow(const string& src, vector<string>& res, string delim = "|") {
    size_t size = src.size();
    size_t pos = 0;
    for (int i = 0; i < size; ++i) {
        pos = src.find(delim, i);
        if (pos < size) {
            string s = src.substr(i, pos - i);
            res.push_back(s);
            i = pos + delim.size() - 1;
        }
    }
}

const string start_date = "1992-01-01";
const string end_date = "1998-12-31";

uint32_t gapDays(const string& date1, const string& date2) {
    std::tm tm1, tm2;
    std::memset(&tm1, 0, sizeof(tm1));
    std::memset(&tm2, 0, sizeof(tm2));
    strptime(date1.data(), "%Y-%m-%d", &tm1);
    strptime(date2.data(), "%Y-%m-%d", &tm2);
    uint32_t gapSecs = static_cast<uint32_t>(std::difftime(std::mktime(&tm2), std::mktime(&tm1)));
    return gapSecs / 86400;
}

uint32_t uniformDate(const string& cur_date) {
    uint32_t all_days = gapDays(start_date, end_date);
    uint32_t cur_days = gapDays(start_date, cur_date);
    return cur_days * 1000 / all_days;
} 

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distribution(1, 999);

void EncodeRow(const vector<string>& row, string* key, string* value) {
    // Encode key
    key->append(fill_key(row[l_orderkey], 8));
    key->append(fill_key(row[l_linenumber], 8));

    // uint64_t orderkey = 0, linenumber = 0;
    // orderkey = std::stoull(row[l_orderkey].c_str(), nullptr, 10);
    // linenumber = std::stoull(row[l_linenumber].c_str(), nullptr, 10);
    // PutFixed64(key, orderkey);
    // PutFixed64(key, linenumber);
    // cout << "row[l_orderkey]: " << row[l_orderkey] << ", row[l_linenumber]: " << row[l_linenumber] << endl;
    // cout << "orderkey: " << orderkey << ", linenumer: " << linenumber << endl;

    // Encode values

    // [shipdate, quantity, l_receiptdate]作为前三个属性
    uint32_t shipdate = 0;
    shipdate = uniformDate(row[l_shipdate].c_str());
    PutFixed32(value, shipdate);

    uint32_t quantity = 0;
    quantity = std::stoul(row[l_quantity].c_str(), nullptr, 10);
    // quantity = distribution(gen);
    PutFixed32(value, quantity);

    uint32_t receiptdate = 0;
    receiptdate = uniformDate(row[l_receiptdate].c_str());
    PutFixed32(value, receiptdate);

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
    PutFixed64(value, discount);

    double tax = 0;
    tax = std::atof(row[l_tax].c_str());
    PutFixed64(value, tax);

    char returnflag = ' ';
    returnflag = row[l_returnflag][0];
    PutLengthPrefixedSlice(value, {&returnflag, 1});

    char linestatus = ' ';
    linestatus = row[l_linestatus][0];
    PutLengthPrefixedSlice(value, {&linestatus, 1});

    uint32_t commitdate = 0;
    commitdate = uniformDate(row[l_commitdate].c_str());
    PutFixed32(value, commitdate);

    string shipinstruct = fill_value(row[l_shipinstruct], 64);
    PutLengthPrefixedSlice(value, shipinstruct);

    string shipmode = fill_value(row[l_shipmode], 64);
    PutLengthPrefixedSlice(value, shipmode);

    string comment = fill_value(row[l_comment], 128);
    PutLengthPrefixedSlice(value, comment);
}

constexpr size_t shipdate_offset = 0;
constexpr size_t quantity_offset = 4;
constexpr size_t discount_offset = 36;


// build secondary index
// shipdate + quantity + receiptdate + pk as secondary index
void encodeSecondaryKey(const vector<string>& row, string* secondary_key) {
    // encode index
    std::string index_key = fill_value("index", 8);
    PutLengthPrefixedSlice(secondary_key, index_key);


    uint32_t shipdate = 0;
    shipdate = uniformDate(row[l_shipdate].c_str());
    PutFixed32(secondary_key, shipdate);

    uint32_t quantity = 0;
    quantity = std::stoul(row[l_quantity].c_str(), nullptr, 10);
    PutFixed32(secondary_key, quantity);

    uint32_t receiptdate = 0;
    receiptdate = uniformDate(row[l_receiptdate].c_str());
    PutFixed32(secondary_key, receiptdate);

    // Encode key
    secondary_key->append(fill_key(row[l_orderkey], 8));
    secondary_key->append(fill_key(row[l_linenumber], 8));

    std::cout << "shipdate: " << shipdate << ", quantity: " << quantity << ", receiptdate: " << receiptdate << ", orderkey: " << row[l_orderkey] << ", linenumber: " << row[l_linenumber] << std::endl;
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
    int num_iteration;
    string input_filename, db_location;
    bool print_file_info, evict, unlimit_fd;
    uint32_t start1, end1, start2, end2;

    int query_type;
    bool predefined_range;
    bool use_secondary_index;

    cxxopts::Options commandline_options("leveldb tpch test", "Testing leveldb based on tpch lineitem.");
    commandline_options.add_options()
            ("f,input_file", "the filename of input file", cxxopts::value<string>(input_filename)->default_value(""))
            ("m,modification", "if set, run our modified version", cxxopts::value<int>(adgMod::MOD)->default_value("0"))
            ("i,iteration", "the number of iterations of a same size", cxxopts::value<int>(num_iteration)->default_value("0"))
            ("h,help", "print help message", cxxopts::value<bool>()->default_value("false"))
            ("d,directory", "the directory of db", cxxopts::value<string>(db_location)->default_value("/mnt/doux/testdb"))
            ("init_db", "init database", cxxopts::value<bool>(fresh_write)->default_value("false"))
            ("c,uncache", "evict cache", cxxopts::value<bool>(evict)->default_value("false"))
            ("file_info", "print the file structure info", cxxopts::value<bool>(print_file_info)->default_value("true"))
            ("unlimit_fd", "unlimit fd", cxxopts::value<bool>(unlimit_fd)->default_value("false"))
            ("dummy", "dummy option")
            ("t, query_type", "the query type of tpch", cxxopts::value<int>(query_type)->default_value("6"))
            ("predefined_range", "use predefined range", cxxopts::value<bool>(predefined_range)->default_value("false"))
            ("si", "use secondary index", cxxopts::value<bool>(use_secondary_index)->default_value("false"))
            ("filter", "use filter", cxxopts::value<bool>(adgMod::use_filter)->default_value("false"))
            ("start1", "start for attribute 1", cxxopts::value<uint32_t>(start1)->default_value("1"))
            ("end1", "end for attribute 1", cxxopts::value<uint32_t>(end1)->default_value("10"))
            ("start2", "start for attribute 2", cxxopts::value<uint32_t>(start2)->default_value("1"))
            ("end2", "end for attribute 2", cxxopts::value<uint32_t>(end2)->default_value("10"));

    auto result = commandline_options.parse(argc, argv);
    if (result.count("help")) {
        printf("%s", commandline_options.help().c_str());
        exit(0);
    }
    
    adgMod::key_size = 16;
    adgMod::value_size = 314;
    adgMod::fd_limit = unlimit_fd ? 1024 * 1024 : 1024;
    DB* db;
    Options options;
    ReadOptions& read_options = adgMod::read_options;
    WriteOptions& write_options = adgMod::write_options;
    Status status;

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

    cout << "input_filename: " << input_filename << ", fresh_write: " << fresh_write << endl;
    if (!input_filename.empty() && fresh_write) {
        string command = "rm -rf " + db_location;
        system(command.c_str());
        system("sync; echo 3 | tee /proc/sys/vm/drop_caches");
        cout << "delete and trim complete" << endl;

        status = DB::Open(options, db_location, &db);
        assert(status.ok() && "Open Error");

        ifstream input(input_filename);
        string src;
        vector<string> row;
        while (getline(input, src)) {
            // cout << src << endl;
            splitRow(src, row);
            // for (int i = 0; i < row.size(); i++) {
            //     cout << row[i] << endl;
            // }
            // cout << endl;
            string key, value;
            EncodeRow(row, &key, &value);
            // cout << "key: " << key.size() << ", value: " << value.size() << endl; 

            instance->StartTimer(9);
            if(adgMod::MOD <= 10) {
                status = db->Put(write_options, key, value);
                // if use secondary index
                if(use_secondary_index) {
                    string secondary_key;
                    encodeSecondaryKey(row, &secondary_key);
                    status = db->Put(write_options, secondary_key, key);

                    // std::cout << "secondary key size: " << secondary_key.size() << std::endl;
                    // std::cout << "secondary index key: " << secondary_key << ", primary key: " << key << std::endl;
                }
            } else {
                // status = db->sPut(write_options, key, , value);
            }
            


            assert(status.ok() && "File Put Error");
            instance->PauseTimer(9, true);

            row.clear();
        }

        cout << "Put Complete" << endl;
        if (print_file_info) {
            db->PrintFileInfo();
        }
        adgMod::db->WaitForBackground();
        delete db;
    }

    
    for (size_t iteration = 0; iteration < num_iteration; ++iteration) {
        if (evict) {
            system("sync; echo 3 | tee /proc/sys/vm/drop_caches");
            cout << "Drop all caches" << endl;
        }

        // cout << "Starting up!" << endl;
        // cout << "Dim 1 from " << read_options.start1 << " to " << read_options.end1 << endl;
        // cout << "Dim 2 from " << read_options.start2 << " to " << read_options.end2 << endl; 

        // TPC-h Q6
        if(query_type == 6) {
            cout << "Begin Test TPC-H Q6!\n";
            status = DB::Open(options, db_location, &db);
            assert(status.ok() && "Open Error");
            adgMod::db->WaitForBackground();

            Iterator* db_iter;
            uint64_t res_count = 0;
            instance->StartTimer(13);
        
            if (adgMod::MOD == 8) {
                // Wisckey
                if(use_secondary_index) {
                    
                } else {
                    // 没有secondary index，需要全表扫描
                    db_iter = db->NewIterator(read_options);
                    instance->StartTimer(4);
                    db_iter->SeekToFirst();
                    instance->PauseTimer(4);

                    instance->StartTimer(17);
                    for (; db_iter->Valid(); db_iter->Next()) {
                        Slice value = db_iter->value();
                        Slice key = db_iter->key();

                        uint32_t attr1 = DecodeFixed32(value.data());
                        uint32_t attr2 = DecodeFixed32(value.data() + sizeof(uint32_t));
                        if (start1 <= attr1 && attr1 <= end1 && start2 <= attr2 && attr2 <= end2) {
                            ++res_count;
                        }
                    }
                    instance->PauseTimer(17);
                }
            } else if (adgMod::MOD == 9) {
                db_iter = db->NewVIterator(read_options);
                instance->StartTimer(4);
                db_iter->Seek(Slice());
                instance->PauseTimer(4);

                instance->StartTimer(17);
                for (; db_iter->Valid(); db_iter->Next()) {
                    Slice key = db_iter->key();
                    Slice value = db_iter->value();
                    ++res_count;
                }
                instance->PauseTimer(17);
            } else if (adgMod::MOD == 10) {
                uint64_t scan_len = 0;
                cout << "Morton code from " << read_options.start << " to "  << read_options.end << endl;
                for (const auto interval : region.intervals_) {
                    cout << "interval from " << interval.start_ << " to " << interval.end_ << endl;
                    scan_len += interval.end_ - interval.start_;
                }
                cout << "scan length: " << scan_len << endl;
                db_iter = db->NewVIterator(read_options);
                instance->StartTimer(4);
                db_iter->Seek(Slice());
                instance->PauseTimer(4);

                instance->StartTimer(17);
                for (; db_iter->Valid(); db_iter->Next()) {
                    Slice key = db_iter->key();
                    Slice value = db_iter->value();
                    ++res_count;
                }
                instance->PauseTimer(17);
            } else {
                // LevelDB

                uint32_t shipdate_start, shipdate_end;
                double discount_start, discount_end;
                uint32_t quantity_start, quantity_end;
                if(!predefined_range) {
                    // if no predefined range, use typical tpch query range
                    shipdate_start  = uniformDate("1993-01-01");
                    shipdate_end    = uniformDate("1994-01-01");
                    
                    discount_start  = 0.03;
                    discount_end    = 0.05;

                    quantity_start  = 0;
                    quantity_end    = 24;
                }
                std::cout << "shipdate_start: " << shipdate_start << ", shipdate_end: " << shipdate_end << std::endl;
                std::cout << "discount_start: " << discount_start << ", discount_end: " << discount_end << std::endl;
                std::cout << "quantity_start: " << quantity_start << ", quantity_end: " << quantity_end << std::endl;
                
                if(use_secondary_index) {
                    // use secondary_index to find the primary key first

                    string secondary_start, secondary_end;
                    /*
                        calculate the secondary start and secondary end based on shipdate, discount, and quantity
                    */
                    {
                        std::vector<string> start_row;
                        start_row.resize(20);
                        start_row[l_shipdate] = "1993-01-01";
                        start_row[l_quantity] = "0";
                        start_row[l_receiptdate] = "1992-01-01";
                        start_row[l_orderkey] = "0";
                        start_row[l_linenumber] = "0";
                        encodeSecondaryKey(start_row, &secondary_start);

                        std::vector<string> end_row;
                        end_row.resize(20);
                        end_row[l_shipdate] = "1994-01-01";
                        end_row[l_quantity] = "23";
                        end_row[l_receiptdate] = "1997-12-31";
                        end_row[l_orderkey] = "99999999";
                        end_row[l_linenumber] = "99999999";
                        encodeSecondaryKey(end_row, &secondary_end);
                    }
                    
                    std::cout << "secondary start size: " << secondary_start.size() << ", secondary end size: " << secondary_end.size() << std::endl;
                    std::cout << "secondary start: " << secondary_start << ", secondary_end: " << secondary_end << std::endl;

                    db_iter = db->NewIterator(read_options);
                    std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;

                    instance->StartTimer(4);
                    db_iter->Seek(secondary_start);
                    instance->PauseTimer(4);

                    std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;

                    if (db_iter->Valid()) {
                        std::cout << "First key after Seek: " << db_iter->key().ToString() << std::endl;
                    } else {
                        std::cout << "No valid key found after Seek." << std::endl;
                    }

                    std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;

                    instance->StartTimer(17);
                    for (; db_iter->Valid(); db_iter->Next()) {
                        Slice secondary_key = db_iter->key();
                        Slice primary_key = db_iter->value();

                        // std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;


                        string value;
                        Status s = db->Get(ReadOptions(), primary_key, &value);

                        std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;

                        // if find the value
                        if(s.ok()) {
                            uint32_t shipdate = DecodeFixed32(value.data() + shipdate_offset);
                            double discount = DecodeFixed64(value.data() + discount_offset);
                            uint32_t quantity = DecodeFixed32(value.data() + quantity_offset);

                            // std::cout << "shipdate: " << shipdate << ", discount: " << discount << ", quantity: " << quantity << std::endl;
                            if(shipdate >= shipdate_start && shipdate < shipdate_end  && quantity < quantity_end) {
                                ++res_count;
                            }
                        }

                    }
                    instance->PauseTimer(17);


                }else {
                    // no secondary index, need full table scan

                    db_iter = db->NewIterator(read_options);
                    instance->StartTimer(4);
                    db_iter->SeekToFirst();
                    instance->PauseTimer(4);
                    
                    instance->StartTimer(17);
                    for (; db_iter->Valid(); db_iter->Next()) {
                        Slice key = db_iter->key();
                        Slice value = db_iter->value();

                        uint32_t shipdate = DecodeFixed32(value.data() + shipdate_offset);
                        double discount = DecodeFixed64(value.data() + discount_offset);
                        uint32_t quantity = DecodeFixed32(value.data() + quantity_offset);

                        // std::cout << "shipdate: " << shipdate << ", discount: " << discount << ", quantity: " << quantity << std::endl;
                        if(shipdate >= shipdate_start && shipdate < shipdate_end  && quantity < quantity_end) {
                            ++res_count;
                        }
                    }
                    instance->PauseTimer(17);
                }
            }
            instance->PauseTimer(13, true);
            cout << "Num of entries within range: " << res_count << endl;
            delete db_iter;
        }
    
        /*
            report running message
        */
        instance->ReportTime();
        for (int s = 0; s < times.size(); ++s) {
            times[s].push_back(instance->ReportTime(s));
        }
        adgMod::db->WaitForBackground();
        sleep(10);

        for (auto& event_array : events) {
            for (Event* e : event_array) e->Report();
        }

        for (Counter& c : levelled_counters) {
            c.Report();
        }
        file_data->Report();

        for (auto it : file_stats) {
            printf("FileStats %d %d %lu %lu %u %u %lu %d\n", it.first, it.second.level, it.second.start,
                it.second.end, it.second.num_lookup_pos, it.second.num_lookup_neg, it.second.size, it.first < file_data->watermark ? 0 : 1);
        }
        delete db;
    }

    for (int s = 0; s < times.size(); ++s) {
        vector<uint64_t>& time = times[s];
        vector<double> diff(time.size());
        if (time.empty()) continue;

        double sum = std::accumulate(time.begin(), time.end(), 0.0);
        double mean = sum / time.size();
        std::transform(time.begin(), time.end(), diff.begin(), [mean] (double x) { return x - mean; });
        double stdev = std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / time.size());

        printf("Timer %d MEAN: %lu, STDDEV: %f\n", s, (uint64_t) mean, stdev);
    }

    if (num_iteration > 1) {
        cout << "Data Without the First Item" << endl;
        for (int s = 0; s < times.size(); ++s) {
            vector<uint64_t>& time = times[s];
            vector<double> diff(time.size() - 1);
            if (time.empty()) continue;

            double sum = std::accumulate(time.begin() + 1, time.end(), 0.0);
            double mean = sum / (time.size() - 1);
            std::transform(time.begin() + 1, time.end(), diff.begin(), [mean] (double x) { return x - mean; });
            double stdev = std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0) / time.size());

            printf("Timer %d MEAN: %lu, STDDEV: %f\n", s, (uint64_t) mean, stdev);
        }
    }

    return 0;
}