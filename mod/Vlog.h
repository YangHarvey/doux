#ifndef LEVELDB_VLOG_H
#define LEVELDB_VLOG_H

#include <mutex>

#include "leveldb/env.h"

using namespace leveldb;

namespace adgMod {

class VLog {
friend class GroupValueLog;
friend class DBImpl;
private:
    std::string vlog_name;

    WritableFile* writer;
    RandomAccessFile* reader;
    std::string buffer;
    uint64_t vlog_size;
    uint64_t curr_addr;
    char* scratch;

    std::mutex log_mutex;

    void Flush();
    void Validate();



public:
    explicit VLog(const std::string& vlog_name);
    uint64_t AddRecord(const Slice& key, const Slice& value);
    uint64_t AddRecord2(const Slice& key, const Slice& value); // 定长record
    std::string ReadRecord(uint64_t address, uint32_t size);
    Slice ReadRecord2(uint64_t address, uint32_t size);
    void Sync();
    // reset write file
    void Reset();

    uint64_t getVlogsize() { return vlog_size; }
    ~VLog();
};

}




#endif //LEVELDB_VLOG_H
