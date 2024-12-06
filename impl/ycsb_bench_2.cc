#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <random>
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "../mod/util.h"
#include "../mod/stats.h"
#include "../mod/cxxopts.hpp"
#include "../db/version_set.h"
#include "zipfian.h"

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

int num_pairs_base = 1024;

class NumericalComparator : public Comparator {
public:
    NumericalComparator() = default;
    virtual const char* Name() const {return "adgMod:NumericalComparator";}
    virtual int Compare(const Slice& a, const Slice& b) const {
        uint64_t ia = adgMod::ExtractInteger(a.data(), a.size());
        uint64_t ib = adgMod::ExtractInteger(b.data(), b.size());
        if (ia < ib) return -1;
        else if (ia == ib) return 0;
        else return 1;
    }
    virtual void FindShortestSeparator(std::string* start, const Slice& limit) const { return; };
    virtual void FindShortSuccessor(std::string* key) const { return; };
};

enum LoadType {
    Ordered = 0,
    Reversed = 1,
    ReversedChunk = 2,
    Random = 3,
    RandomChunk = 4
};

enum WorkloadType {
    Insert = 0,
    Update = 1,
    Get = 2,
    Scan = 3,
    YCSB_A = 4,
    YCSB_B = 5,
    YCSB_C = 6,
    YCSB_D = 7,
    YCSB_E = 8,
    YCSB_F = 9
};

// Regenerate keys after init db
// Existing keys: [0, num_entries - 1]
// Beyond existing keys: [num_entries, 999,999,999,999,999] (within 16 bytes)
void fillKeysByWorkloads(vector<string>& keys, int workload_type, uint64_t num_entries, uint64_t num_operations, int load_type) {
    std::default_random_engine e1(255);
    std::uniform_int_distribution<uint64_t> udist_within_key(0, num_entries - 1);
    std::uniform_int_distribution<uint64_t> udist_beyond_key(num_entries, 999999999999999);
    zipfian_int_distribution<uint64_t> zipf_dist_within_key(0, num_entries - 1, 0.9);
    zipfian_int_distribution<uint64_t> zipf_dist_beyond_key(num_entries, num_entries + num_operations, 0.9);
    keys.reserve(num_operations);

    switch (workload_type) {
        case WorkloadType::Insert: {
            for (uint64_t i = 0; i < num_operations; ++i) {
                keys.push_back(generate_key(to_string(udist_beyond_key(e1))));
            }
            break;
        }
        case WorkloadType::Update:
        case WorkloadType::Get:
        case WorkloadType::Scan: {
            for (uint64_t i = 0; i < num_operations; ++i) {
                keys.push_back(generate_key(to_string(udist_within_key(e1))));
            }
            break;
        }
        case WorkloadType::YCSB_A:
        case WorkloadType::YCSB_B:
        case WorkloadType::YCSB_C: {
            for (uint64_t i = 0; i < num_operations; ++i) {
                keys.push_back(generate_key(to_string(zipf_dist_within_key(e1))));
            }
            break;
        }
        case WorkloadType::YCSB_D:
        case WorkloadType::YCSB_E: {
            uint64_t insert_keys = num_operations * 0.05;
            for (uint64_t i = 0; i < insert_keys; ++i) {
                keys.push_back(generate_key(to_string(zipf_dist_beyond_key(e1))));
            }
            for (uint64_t i = insert_keys; i < num_operations; ++i) {
                keys.push_back(generate_key(to_string(zipf_dist_within_key(e1))));
            }
            break;
        }
        case WorkloadType::YCSB_F: {
            for (uint64_t i = 0; i < num_operations; ++i) {
                keys.push_back(generate_key(to_string(zipf_dist_within_key(e1))));
            }
            break;
        }
        default: assert(false && "Unsupported workload type.");
    }

    if (load_type == LoadType::Ordered) {
        for (uint64_t i = 0; i < num_operations; i++) {
            keys[i] = generate_key(to_string(i));
        }
    }

    std::cout << "Conduct ops num: " << num_operations << std::endl;
}

int main(int argc, char *argv[]) {
    uint64_t num_entries, num_operations;
    int num_iteration;
    float num_pair_lower, num_pair_upper, num_pair_step;
    string db_location, workload;
    bool print_single_timing, print_file_info, evict, unlimit_fd;
    int workload_type, load_type, length_range;

    cxxopts::Options commandline_options("leveldb ycsb test", "Testing leveldb based on YCSB.");
    commandline_options.add_options()
            ("e,num_entries", "the number of entries (to be multiplied by 1024)", cxxopts::value<uint64_t>(num_entries)->default_value("1024"))
            ("n,ops_number", "the number of operations (to be multiplied by 1024)", cxxopts::value<uint64_t>(num_operations)->default_value("1024"))
            ("w,workload", "test worklad", cxxopts::value<int>(workload_type)->default_value("2"))
            ("s,step", "the step of the loop of the size of db", cxxopts::value<float>(num_pair_step)->default_value("1"))
            ("i,iteration", "the number of iterations of a same size", cxxopts::value<int>(num_iteration)->default_value("1"))
            ("m,modification", "if set, run our modified version", cxxopts::value<int>(adgMod::MOD)->default_value("0"))
            ("h,help", "print help message", cxxopts::value<bool>()->default_value("false"))
            ("d,directory", "the directory of db", cxxopts::value<string>(db_location)->default_value("/mnt/doux/testdb"))
            ("k,key_size", "the size of key", cxxopts::value<int>(adgMod::key_size)->default_value("8"))
            ("v,value_size", "the size of value", cxxopts::value<int>(adgMod::value_size)->default_value("8"))
            ("single_timing", "print the time of every single get", cxxopts::value<bool>(print_single_timing)->default_value("false"))
            ("file_info", "print the file structure info", cxxopts::value<bool>(print_file_info)->default_value("true"))
            ("multiple", "test: use larger keys", cxxopts::value<uint64_t>(adgMod::key_multiple)->default_value("1"))
            ("init_db", "init database", cxxopts::value<bool>(fresh_write)->default_value("false"))
            ("c,uncache", "evict cache", cxxopts::value<bool>(evict)->default_value("false"))
            ("unlimit_fd", "unlimit fd", cxxopts::value<bool>(unlimit_fd)->default_value("false"))
            ("x,dummy", "dummy option")
            ("t,load_type", "load type", cxxopts::value<int>(load_type)->default_value("0"))
            ("filter", "use filter", cxxopts::value<bool>(adgMod::use_filter)->default_value("false"))
            ("use_dropmap", "if use drop map", cxxopts::value<bool>(adgMod::use_dropmap)->default_value("true"))
            ("decoupled_compaction", "if use decoupled compaction", cxxopts::value<bool>(adgMod::if_decoupled_compaction)->default_value("true"))
            ("range", "use range query and specify length", cxxopts::value<int>(length_range)->default_value("100"));

    auto result = commandline_options.parse(argc, argv);
    if (result.count("help")) {
        printf("%s", commandline_options.help().c_str());
        exit(0);
    }

    std::default_random_engine e1(255), e2(0);
    srand(0);
    num_entries *= num_pairs_base;
    num_operations *= num_pairs_base;

    adgMod::fd_limit = unlimit_fd ? 1024 * 1024 : 1024;

    vector<string>& keys = adgMod::keys;
    if (fresh_write) {
        // keys: [0, num_entries - 1]
        for (uint64_t i = 0; i < num_entries; ++i) {
            keys.push_back(generate_key(to_string(i)));
        }
        cout << "Generate keys num: " << keys.size() << endl;
    }

    adgMod::Stats* instance = adgMod::Stats::GetInstance();
    vector<vector<size_t>> times(20);
    string values(1024 * 1024, '0');
    for (size_t iteration = 0; iteration < num_iteration; ++iteration) {
        std::uniform_int_distribution<uint64_t> uniform_dist_value(0, (uint64_t) values.size() - adgMod::value_size - 1);

        DB* db;
        Options options;
        ReadOptions& read_options = adgMod::read_options;
        WriteOptions& write_options = adgMod::write_options;
        Status status;

        options.create_if_missing = true;
        write_options.sync = false;
        instance->ResetAll();

        if (fresh_write && iteration == 0) {
            string command = "rm -rf " + db_location;
            system(command.c_str());
            system("sync; echo 3 | tee /proc/sys/vm/drop_caches");

            status = DB::Open(options, db_location, &db);
            assert(status.ok() && "Open Error");


            /*
                load data
            */
            instance->StartTimer(9);
            int cut_size = keys.size() / 100000;
            std::vector<std::pair<int, int>> chunks;
            switch (load_type) {
                case LoadType::Ordered: {
                    for (int cut = 0; cut < cut_size; ++cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case LoadType::ReversedChunk: {
                    for (int cut = cut_size - 1; cut >= 0; --cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case LoadType::Random: {
                    std::random_shuffle(keys.begin(), keys.end());
                    for (int cut = 0; cut < cut_size; ++cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case LoadType::RandomChunk: {
                    for (int cut = 0; cut < cut_size; ++cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    std::random_shuffle(chunks.begin(), chunks.end());
                    break;
                }
                default: assert(false && "Unsupported load type.");
            }

            for (int cut = 0; cut < chunks.size(); ++cut) {
                for (int i = chunks[cut].first; i < chunks[cut].second; ++i) {
                    string value = generate_value(uniform_dist_value(e1));
                    status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                    adgMod::put_idx.emplace_back(i);
                }
            }
            adgMod::db->vlog->Sync();
            instance->PauseTimer(9, true);
            cout << "Put Complete" << endl;

            // instance->StartTimer(19);
            if(adgMod::MOD == 12) {
                // RISE need colocationGC
                db->runAllColocationGC();
            }
            // instance->PauseTimer(19, true);

            keys.clear();
            adgMod::db->WaitForBackground();
            if (print_file_info && iteration == 0) db->PrintFileInfo();
            delete db;
            db = nullptr;
        }

        if (evict) {
            system("sync; echo 3 | tee /proc/sys/vm/drop_caches");
            cout << "Drop all caches" << endl;
        }

        
        /*
            exec workload
        */
        if (!fresh_write || iteration > 0) {
            // Prepare keys for later workloads
            keys.clear();
            fillKeysByWorkloads(keys, workload_type, num_entries, num_operations, load_type);

            cout << "Starting up" << endl;
            status = DB::Open(options, db_location, &db);
            adgMod::db->WaitForBackground();
            Iterator* db_iter = length_range == 0 ? nullptr : db->NewIterator(read_options);
            assert(status.ok() && "Open Error");

            uint64_t last_read = 0, last_write = 0;
            int last_level = 0, last_file = 0, last_baseline = 0, last_succeeded = 0, last_false = 0, last_compaction = 0, last_learn = 0;
            std::vector<uint64_t> detailed_times;
            bool start_new_event = true;

            std::cout << "This line is number: " << __FILE__  << ":" << __LINE__ << std::endl;

            instance->StartTimer(13);
            for (int i = 0; i < num_operations; ++i) {
                if (start_new_event) {
                    detailed_times.push_back(instance->GetTime());
                    start_new_event = false;
                }

                // Call interface according to workloads
                switch (workload_type) {
                    case WorkloadType::Insert:
                    case WorkloadType::Update: {
                        // Put
                        string value = generate_value(uniform_dist_value(e2));
                        instance->StartTimer(10);
                        status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                        instance->PauseTimer(10);
                        adgMod::put_idx.emplace_back(i);
                        break;
                    }
                    case WorkloadType::Get: {
                        // Get
                        string value;
                        instance->StartTimer(4);
                        status = db->Get(read_options, keys[i], &value);
                        instance->PauseTimer(4);
                        if (!status.ok()) {
                            cout << keys[i] << " Not Found" << endl;
                        }
                        break;
                    }
                    case WorkloadType::Scan: {
                        // Seek
                        instance->StartTimer(4);
                        db_iter->Seek(keys[i]);
                        instance->PauseTimer(4);

                        // Scan
                        instance->StartTimer(17);
                        for (int r = 0; r < length_range; ++r) {
                            if (!db_iter->Valid()) {
                                std::cout << "cur range: " << r << std::endl;
                                break;
                            }
                            Slice key = db_iter->key();
                            string value = db_iter->value().ToString();
                            db_iter->Next();
                        }
                        instance->PauseTimer(17);
                        break;
                    }
                    case WorkloadType::YCSB_A: {
                        // Update Heavy
                        // 50% updates, 50% reads
                        if (i > 0.5 * num_operations) {
                            string value = generate_value(uniform_dist_value(e2));
                            instance->StartTimer(10);
                            status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                            instance->PauseTimer(10);
                            adgMod::put_idx.emplace_back(i);
                        } else {
                            string value;
                            instance->StartTimer(4);
                            status = db->Get(read_options, keys[i], &value);
                            instance->PauseTimer(4);
                            if (!status.ok()) {
                                cout << keys[i] << " Not Found" << endl;
                            }
                        }
                        break;
                    }
                    case WorkloadType::YCSB_B: {
                        // Read Mostly
                        // 5% updates, 95% reads
                        if (i < 0.05 * num_operations) {
                            string value = generate_value(uniform_dist_value(e2));
                            instance->StartTimer(10);
                            status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                            instance->PauseTimer(10);
                            adgMod::put_idx.emplace_back(i);
                        } else {
                            string value;
                            instance->StartTimer(4);
                            status = db->Get(read_options, keys[i], &value);
                            instance->PauseTimer(4);
                            if (!status.ok()) {
                                cout << keys[i] << " Not Found" << endl;
                            }
                        }
                        break;
                    }
                    case WorkloadType::YCSB_C: {
                        // Read Only
                        // 100% reads
                        string value;
                        instance->StartTimer(4);
                        status = db->Get(read_options, keys[i], &value);
                        instance->PauseTimer(4);
                        if (!status.ok()) {
                            cout << keys[i] << " Not Found" << endl;
                        }
                        break;
                    }
                    case WorkloadType::YCSB_D: {
                        // Read Latest
                        // 5% inserts, 95% reads
                        if (i < 0.05 * num_operations) {
                            string value = generate_value(uniform_dist_value(e2));
                            instance->StartTimer(10);
                            status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                            instance->PauseTimer(10);
                            adgMod::put_idx.emplace_back(i);
                        } else {
                            string value;
                            instance->StartTimer(4);
                            status = db->Get(read_options, keys[i], &value);
                            instance->PauseTimer(4);
                            if (!status.ok()) {
                                cout << keys[i] << " Not Found" << endl;
                            }
                        }
                        break;
                    }
                    case WorkloadType::YCSB_E: {
                        // Scan Mostly
                        // 5% inserts, 95% scans
                        if (i < 0.05 * num_operations) {
                            string value = generate_value(uniform_dist_value(e2));
                            instance->StartTimer(10);
                            status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                            instance->PauseTimer(10);
                            adgMod::put_idx.emplace_back(i);
                        } else {
                            // Seek
                            instance->StartTimer(4);
                            db_iter->Seek(keys[i]);
                            instance->PauseTimer(4);

                            // Scan
                            instance->StartTimer(17);
                            for (int r = 0; r < length_range; ++r) {
                                if (!db_iter->Valid()) break;
                                Slice key = db_iter->key();
                                string value = db_iter->value().ToString();
                                db_iter->Next();
                            }
                            instance->PauseTimer(17);
                        }
                        break;
                    }
                    case WorkloadType::YCSB_F: {
                        // Read-Modify-Write
                        // 50% read-modify-write, 50% reads
                        if (i < 0.5 * num_operations) {
                            // Read
                            string value;
                            instance->StartTimer(4);
                            status = db->Get(read_options, keys[i], &value);
                            instance->PauseTimer(4);
                            if (!status.ok()) {
                                cout << keys[i] << " Not Found" << endl;
                            }

                            // Modify-Write
                            value = generate_value(uniform_dist_value(e2));
                            instance->StartTimer(10);
                            status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                            instance->PauseTimer(10);
                            adgMod::put_idx.emplace_back(i);
                        } else {
                            string value;
                            instance->StartTimer(4);
                            status = db->Get(read_options, keys[i], &value);
                            instance->PauseTimer(4);
                            if (!status.ok()) {
                                cout << keys[i] << " Not Found" << endl;
                            }
                        }
                        break;
                    }
                    default: assert(false && "Unsupported workload type.");
                }
                
                // Stat
                if ((i + 1) % (num_operations / 100) == 0) {
                    detailed_times.push_back(instance->GetTime());
                }
                if ((i + 1) % (num_operations / 10) == 0) {
                    int level_read = levelled_counters[0].Sum();
                    int file_read = levelled_counters[1].Sum();
                    int baseline_read = levelled_counters[2].Sum();
                    int succeeded_read = levelled_counters[3].NumSum();
                    int false_read = levelled_counters[4].NumSum();

                    compaction_counter_mutex.Lock();
                    int num_compaction = events[0].size();
                    compaction_counter_mutex.Unlock();

                    uint64_t read_time = instance->ReportTime(4);
                    uint64_t write_time = instance->ReportTime(10);
                    std::pair<uint64_t, uint64_t> time = {detailed_times.front(), detailed_times.back()};

                    events[2].push_back(new WorkloadEvent(time, level_read - last_level, file_read - last_file, baseline_read - last_baseline,
                        succeeded_read - last_succeeded, false_read - last_false, num_compaction - last_compaction, 0,
                        read_time - last_read, write_time - last_write, std::move(detailed_times)));

                    last_level = level_read;
                    last_file = file_read;
                    last_baseline = baseline_read;
                    last_succeeded = succeeded_read;
                    last_false = false_read;
                    last_compaction = num_compaction;
                    last_read = read_time;
                    last_write = write_time;
                    detailed_times.clear();
                    start_new_event = true;
                    cout << (i + 1) / (num_operations / 10) << endl;
                    Version* current = adgMod::db->versions_->current();
                    printf("LevelSize %d %d %d %d %d %d\n", current->NumFiles(0), current->NumFiles(1), current->NumFiles(2), current->NumFiles(3),
                        current->NumFiles(4), current->NumFiles(5));
                }
            }
            instance->PauseTimer(13, true);

            instance->ReportTime();
            delete db_iter;

            adgMod::db->WaitForBackground();
            sleep(10);
        }

        for (int s = 0; s < times.size(); ++s) {
            times[s].push_back(instance->ReportTime(s));
        }

        for (auto& event_array : events) {
            for (Event* e : event_array) e->Report();
        }

        for (Counter& c : levelled_counters) {
            c.Report();
        }

        for (auto it : file_stats) {
            printf("FileStats %d %d %lu %lu %u %u %lu %d\n", it.first, it.second.level, it.second.start,
                it.second.end, it.second.num_lookup_pos, it.second.num_lookup_neg, it.second.size, it.first < file_data->watermark ? 0 : 1);
        }
        if (db) {
            delete db;
        }
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
    std::cout << "adgmod::drop_count_gain = " << adgMod::drop_count_gain << std::endl;

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

    std::cout << "adgmod::drop_map_size = " << adgMod::drop_map_size << std::endl;
    std::cout << "adgmod::drop_count_gain = " << adgMod::drop_count_gain << std::endl;
    std::cout << "adgMod::redirect_count = " << adgMod::redirect_count << std::endl;
    std::cout << "adgMod::direct_count = " << adgMod::direct_count << std::endl;
}