#ifndef LEVELDB_VLOG_H
#define LEVELDB_VLOG_H

#include "leveldb/env.h"

using namespace leveldb;

namespace adgMod {

class VLog {
private:
    WritableFile* writer;
    RandomAccessFile* reader;
    std::string buffer;
    uint64_t vlog_size;
    uint64_t curr_addr;
    char* scratch;

    void Flush();
    void Validate();

public:
    explicit VLog(const std::string& vlog_name);
    uint64_t AddRecord(const Slice& key, const Slice& value);
    std::string ReadRecord(uint64_t address, uint32_t size);
    Slice ReadRecord2(uint64_t address, uint32_t size);
    void Sync();
    ~VLog();
};

}




#endif //LEVELDB_VLOG_H
