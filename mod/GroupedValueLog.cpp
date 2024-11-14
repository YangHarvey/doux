#include "GroupedValueLog.h"
#include <iostream>
#include <functional>

namespace adgMod {

GroupValueLog::GroupValueLog(const std::string& log_name, int num_groups)
    : num_groups(num_groups) {
    // Initialize each group's VLog
    for (int i = 0; i < num_groups; ++i) {
        std::string group_log_name = log_name + "_group_" + std::to_string(i);
        group_vlogs.push_back(new VLog(group_log_name));  // Create a new VLog for each group
    }
}

GroupValueLog::~GroupValueLog() {
    // Clean up all group VLogs
    for (auto& vlog : group_vlogs) {
        delete vlog;
    }
}

int GroupValueLog::GetGroupIndex(const leveldb::Slice& key) {
    // Hash the key to determine the group index
    std::hash<std::string> hasher;
    return hasher(key.ToString()) % num_groups;
}

std::pair<int, uint64_t> GroupValueLog::AddRecord(const leveldb::Slice& key, const leveldb::Slice& value) {
    // Determine the group index for the key
    int group_index = GetGroupIndex(key);

    // Add the record to the appropriate VLog and get the address
    uint64_t address = group_vlogs[group_index]->AddRecord(key, value);

    // Return the group index and the address within the VLog
    return {group_index, address};
}

std::string GroupValueLog::ReadRecord(int group_num, uint64_t address, uint32_t size) {
    if (group_num < 0 || group_num >= num_groups) {
        return "";  // Invalid group number
    }
    return group_vlogs[group_num]->ReadRecord(address, size);
}

Slice GroupValueLog::ReadRecord2(int group_num, uint64_t address, uint32_t size) {
    if (group_num < 0 || group_num >= num_groups) {
        return Slice{""};  // Invalid group number
    }
    return group_vlogs[group_num]->ReadRecord2(address, size);
}

void GroupValueLog::Flush(int group_num) {
    if (group_num < 0 || group_num >= num_groups) {
        return;  // Invalid group number
    }
    group_vlogs[group_num]->Flush();
}

void GroupValueLog::FlushAll() {
    for(int i = 0; i < num_groups; i++) {
        group_vlogs[i]->Flush();
    }
}

void GroupValueLog::Reset(int group_num) {
    if (group_num < 0 || group_num >= num_groups) {
        return;  // Invalid group number
    }

    group_vlogs[group_num]->Reset();
}

}  // namespace adgMod
