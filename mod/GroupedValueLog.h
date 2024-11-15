#ifndef GROUP_VLOG_H
#define GROUP_VLOG_H

#include <vector>
#include <string>
#include "Vlog.h"  // Assume this is your existing Vlog header file
#include "leveldb/slice.h"

namespace adgMod {

class GroupValueLog {
public:
    // Constructor: Initialize with the number of groups and log base name
    GroupValueLog(const std::string& log_name, int num_groups = 2000);

    // Destructor: Cleanup all resources
    ~GroupValueLog();

    // Method to add a record (key, value) and return the group number and address
    std::pair<uint32_t, uint64_t> AddRecord(const leveldb::Slice& key, const leveldb::Slice& value);

    // Method to read a record using group number and address
    std::string ReadRecord(int group_num, uint64_t address, uint32_t size);

    Slice ReadRecord2(int group_num, uint64_t address, uint32_t size);

    // Reset method to clear the specified group log
    void Flush(int group_num);

    void FlushAll();

    void Reset(int group_num);

    // Number of groups (VLogs)
    int num_groups;

    // Vector to hold each group's VLog
    std::vector<VLog*> group_vlogs;

    // Helper function to determine the group index based on the key
    uint32_t GetGroupIndex(const leveldb::Slice& key);
};

}  // namespace adgMod

#endif  // GROUP_VLOG_H
