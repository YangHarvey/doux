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

int main(int argc, char *argv[]) {
    int num_operations, num_iteration;
    float num_pair_lower, num_pair_upper, num_pair_step;
    string db_location, workload;
    bool print_single_timing, print_file_info, evict, unlimit_fd;
    int load_type, length_range;

    cxxopts::Options commandline_options("leveldb ycsb test", "Testing leveldb based on YCSB.");
    commandline_options.add_options()
            ("n,ops_number", "the number of operations (to be multiplied by 1024)", cxxopts::value<int>(num_operations)->default_value("1024"))
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
            ("w,write", "writedb", cxxopts::value<bool>(fresh_write)->default_value("false"))
            ("c,uncache", "evict cache", cxxopts::value<bool>(evict)->default_value("false"))
            ("unlimit_fd", "unlimit fd", cxxopts::value<bool>(unlimit_fd)->default_value("false"))
            ("x,dummy", "dummy option")
            ("t,load_type", "load type", cxxopts::value<int>(load_type)->default_value("0"))
            ("filter", "use filter", cxxopts::value<bool>(adgMod::use_filter)->default_value("false"))
            ("range", "use range query and specify length", cxxopts::value<int>(length_range)->default_value("0"));

    auto result = commandline_options.parse(argc, argv);
    if (result.count("help")) {
        printf("%s", commandline_options.help().c_str());
        exit(0);
    }

    std::default_random_engine e1(0), e2(255), e3(0);
    srand(0);
    num_operations *= num_pairs_base;

    vector<string> keys;
    std::uniform_int_distribution<uint64_t> udist_key(0, 999999999999999);
    for (int i = 0; i < num_operations; ++i) {
        keys.push_back(generate_key(to_string(udist_key(e2))));
    }
    cout << "Generate keys num: " << keys.size() << endl;

    adgMod::Stats* instance = adgMod::Stats::GetInstance();
    vector<vector<size_t>> times(20);
    string values(1024 * 1024, '0');
    for (size_t iteration = 0; iteration < num_iteration; ++iteration) {
        std::uniform_int_distribution<uint64_t> uniform_dist_file(0, (uint64_t) keys.size() - 1);
        std::uniform_int_distribution<uint64_t> uniform_dist_file2(0, (uint64_t) keys.size() - 1);
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
            system("sudo fstrim -a -v");
            system("sync; echo 3 | sudo tee /proc/sys/vm/drop_caches");

            status = DB::Open(options, db_location, &db);
            assert(status.ok() && "Open Error");

            instance->StartTimer(9);
            int cut_size = keys.size() / 100000;
            std::vector<std::pair<int, int>> chunks;
            switch (load_type) {
                case Ordered: {
                    for (int cut = 0; cut < cut_size; ++cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case ReversedChunk: {
                    for (int cut = cut_size - 1; cut >= 0; --cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case Random: {
                    std::random_shuffle(keys.begin(), keys.end());
                    for (int cut = 0; cut < cut_size; ++cut) {
                        chunks.emplace_back(keys.size() * cut / cut_size, keys.size() * (cut + 1) / cut_size);
                    }
                    break;
                }
                case RandomChunk: {
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
                    string value = generate_value(uniform_dist_value(e2));
                    status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                }
            }
            // adgMod::db->vlog->Sync();
            instance->PauseTimer(9, true);
            cout << "Put Complete" << endl;

            // keys.clear();
            if (print_file_info && iteration == 0) db->PrintFileInfo();
            adgMod::db->WaitForBackground();
            delete db;
        }

        if (evict) {
            system("sync; echo 3 | sudo tee /proc/sys/vm/drop_caches");
        }

        cout << "Starting up" << endl;
        status = DB::Open(options, db_location, &db);
        adgMod::db->WaitForBackground();
        Iterator* db_iter = length_range == 0 ? nullptr : db->NewIterator(read_options);
        assert(status.ok() && "Open Error");

        uint64_t last_read = 0, last_write = 0;
        int last_level = 0, last_file = 0, last_baseline = 0, last_succeeded = 0, last_false = 0, last_compaction = 0, last_learn = 0;
        std::vector<uint64_t> detailed_times;
        bool start_new_event = true;

        instance->StartTimer(13);
        uint64_t write_i = 0;
        for (int i = 0; i < num_operations; ++i) {
            if (start_new_event) {
                detailed_times.push_back(instance->GetTime());
                start_new_event = false;
            }

            bool write = false;
            if (write) {
                // Write
                instance->StartTimer(10);
                string value = generate_value(uniform_dist_value(e3));
                status = db->Put(write_options, keys[i], {value.data(), (uint64_t) adgMod::value_size});
                instance->PauseTimer(10);
            } else if (length_range != 0) {
                // Seek
                instance->StartTimer(4);
                db_iter->Seek(keys[i]);
                instance->PauseTimer(4);

                // Range Scan
                instance->StartTimer(17);
                for (int r = 0; r < length_range; ++r) {
                    if (!db_iter->Valid()) break;
                    Slice key = db_iter->key();
                    string value = db_iter->value().ToString();
                    db_iter->Next();
                }
                instance->PauseTimer(17);
            } else {
                // Get
                string res;
                instance->StartTimer(4);
                status = db->Get(read_options, keys[i], &res);
                instance->PauseTimer(4);
                if (!status.ok()) {
                    cout << keys[i] << " Not Found" << endl;
                }
            }


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

        delete db_iter;
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

}