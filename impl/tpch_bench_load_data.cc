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
#include <sstream>
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
    
    // 获取统计信息以计算写放大
    string stats;
    if (db->GetProperty("leveldb.stats", &stats)) {
        // 解析统计信息，计算总写入量
        uint64_t total_bytes_written = 0;
        std::istringstream iss(stats);
        string line;
        bool header_found = false;
        
        while (std::getline(iss, line)) {
            if (line.find("Write(MB)") != string::npos) {
                header_found = true;
                continue;
            }
            if (header_found && line.find("---") == string::npos && !line.empty()) {
                // 解析每一行的写入量（MB），格式: "Level  Files Size(MB) Time(sec) Read(MB) Write(MB) Count"
                std::istringstream line_stream(line);
                int level, files;
                double size_mb, time_sec, read_mb, write_mb;
                int64_t count = 0;
                if (line_stream >> level >> files >> size_mb >> time_sec >> read_mb >> write_mb >> count) {
                    total_bytes_written += static_cast<uint64_t>(write_mb * 1048576.0);
                } else if (line_stream >> level >> files >> size_mb >> time_sec >> read_mb >> write_mb) {
                    // 兼容旧格式（没有 Count 列）
                    total_bytes_written += static_cast<uint64_t>(write_mb * 1048576.0);
                }
            }
        }
        
        if (total_bytes_written > 0) {
            double total_write_mb = total_bytes_written / 1048576.0;
            double total_write_gb = total_bytes_written / 1073741824.0;
            std::cout << "=== Write Statistics ===" << std::endl;
            std::cout << "Total bytes written (all levels): " << total_bytes_written << " bytes" << std::endl;
            std::cout << "Total bytes written: " << total_write_mb << " MB" << std::endl;
            std::cout << "Total bytes written: " << total_write_gb << " GB" << std::endl;
            std::cout << "STATS_TOTAL_BYTES_WRITTEN=" << total_bytes_written << std::endl;
            std::cout << std::endl;
            
            // 详细统计每个 level 的写入
            std::cout << "=== Per-Level Write Breakdown ===" << std::endl;
            std::cout << stats << std::endl;
            
            // 解析并显示每个 level 的详细统计
            std::cout << "=== Detailed Write Statistics by Level ===" << std::endl;
            std::istringstream iss2(stats);
            string line2;
            bool header_found2 = false;
            uint64_t level0_write = 0, level1_write = 0, level2_write = 0, other_level_write = 0;
            int64_t level0_count = 0, level1_count = 0, level2_count = 0, other_level_count = 0;
            uint64_t level0_read = 0, level1_read = 0, level2_read = 0, other_level_read = 0;
            
            while (std::getline(iss2, line2)) {
                if (line2.find("Write(MB)") != string::npos || line2.find("Count") != string::npos) {
                    header_found2 = true;
                    continue;
                }
                if (header_found2 && line2.find("---") == string::npos && !line2.empty()) {
                    std::istringstream line_stream2(line2);
                    int level, files;
                    double size_mb, time_sec, read_mb, write_mb;
                    int64_t count = 0;
                    if (line_stream2 >> level >> files >> size_mb >> time_sec >> read_mb >> write_mb >> count) {
                        uint64_t write_bytes = static_cast<uint64_t>(write_mb * 1048576.0);
                        uint64_t read_bytes = static_cast<uint64_t>(read_mb * 1048576.0);
                        if (level == 0) {
                            level0_write += write_bytes;
                            level0_read += read_bytes;
                            level0_count += count;
                        } else if (level == 1) {
                            level1_write += write_bytes;
                            level1_read += read_bytes;
                            level1_count += count;
                        } else if (level == 2) {
                            level2_write += write_bytes;
                            level2_read += read_bytes;
                            level2_count += count;
                        } else {
                            other_level_write += write_bytes;
                            other_level_read += read_bytes;
                            other_level_count += count;
                        }
                    } else if (line_stream2 >> level >> files >> size_mb >> time_sec >> read_mb >> write_mb) {
                        // 兼容旧格式
                        uint64_t write_bytes = static_cast<uint64_t>(write_mb * 1048576.0);
                        uint64_t read_bytes = static_cast<uint64_t>(read_mb * 1048576.0);
                        if (level == 0) {
                            level0_write += write_bytes;
                            level0_read += read_bytes;
                            level0_count += 1;  // 假设至少一次
                        } else if (level == 1) {
                            level1_write += write_bytes;
                            level1_read += read_bytes;
                            level1_count += 1;
                        } else if (level == 2) {
                            level2_write += write_bytes;
                            level2_read += read_bytes;
                            level2_count += 1;
                        } else {
                            other_level_write += write_bytes;
                            other_level_read += read_bytes;
                            other_level_count += 1;
                        }
                    }
                }
            }
            
            std::cout << "Level 0 writes (flush): " << level0_write << " bytes (" 
                      << level0_write / 1048576.0 << " MB), " << level0_count 
                      << " operations, avg " << (level0_count > 0 ? level0_write / level0_count / 1048576.0 : 0) 
                      << " MB per operation" << std::endl;
            std::cout << "Level 1 writes (compaction): " << level1_write << " bytes (" 
                      << level1_write / 1048576.0 << " MB), " << level1_count 
                      << " compactions, avg " << (level1_count > 0 ? level1_write / level1_count / 1048576.0 : 0) 
                      << " MB per compaction" << std::endl;
            if (level2_write > 0) {
                std::cout << "Level 2 writes (compaction): " << level2_write << " bytes (" 
                          << level2_write / 1048576.0 << " MB), " << level2_count 
                          << " compactions, avg " << (level2_count > 0 ? level2_write / level2_count / 1048576.0 : 0) 
                          << " MB per compaction" << std::endl;
            }
            if (other_level_write > 0) {
                std::cout << "Other levels writes: " << other_level_write << " bytes (" 
                          << other_level_write / 1048576.0 << " MB), " << other_level_count 
                          << " compactions" << std::endl;
            }
            std::cout << "Total compaction count: " << (level0_count + level1_count + level2_count + other_level_count) << std::endl;
            std::cout << std::endl;
            
            // 验证统计完整性
            uint64_t sum_by_level = level0_write + level1_write + level2_write + other_level_write;
            if (sum_by_level != total_bytes_written) {
                std::cout << "⚠ Warning: Sum by level (" << sum_by_level 
                          << ") != Total (" << total_bytes_written << ")" << std::endl;
            } else {
                std::cout << "✓ Statistics verification: Sum by level matches total" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    delete db;

    input.close();

    std::cout << "Time taken: " << instance->ReportTime(9) << " seconds" << std::endl;
    return 0;
}