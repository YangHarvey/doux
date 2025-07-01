#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include "Vlog.h"
#include "util.h"
#include "util/coding.h"

using std::string;



const int buffer_size_max = 300 * 1024;

namespace adgMod {

VLog::VLog(const std::string& vlog_name) : vlog_name(vlog_name), writer(nullptr), reader(nullptr) {
    adgMod::env->NewWritableFile(vlog_name, &writer);
    adgMod::env->NewRandomAccessFile(vlog_name, &reader);
    buffer.reserve(buffer_size_max * 2);
    struct ::stat file_stat;
    ::stat(vlog_name.c_str(), &file_stat);
    vlog_size = file_stat.st_size;
    scratch = new char[adgMod::value_size];
}

uint64_t VLog::AddRecord(const Slice& key, const Slice& value) {
    // std::lock_guard<std::mutex> lock(log_mutex);
    PutLengthPrefixedSlice(&buffer, key);
    PutVarint32(&buffer, value.size());
    uint64_t result = vlog_size + buffer.size();
    buffer.append(value.data(), value.size());

    if (buffer.size() >= buffer_size_max) Flush();

    return result;
}

// 定长key
uint64_t VLog::AddRecord2(const Slice& key, const Slice& value) {
    // std::lock_guard<std::mutex> lock(log_mutex);
    PutFixed32(&buffer, key.size());
    buffer.append(key.data(), key.size());
    PutFixed32(&buffer, value.size());
    uint64_t result = vlog_size + buffer.size();
    buffer.append(value.data(), value.size());

    if (buffer.size() >= buffer_size_max) Flush();

    return result;
}

string VLog::ReadRecord(uint64_t address, uint32_t size) {
    // std::lock_guard<std::mutex> lock(log_mutex);
    if (address >= vlog_size) return string(buffer.c_str() + address - vlog_size, size);
    
    Slice value;
    reader->Read(address, size, &value, scratch);
    string result(value.data(), value.size());
    return result;
}

Slice VLog::ReadRecord2(uint64_t address, uint32_t size) {
    // std::lock_guard<std::mutex> lock(log_mutex);
    if (address >= vlog_size) return string(buffer.c_str() + address - vlog_size, size);

    Slice value;
    reader->Read(address, size, &value, scratch);
    return value;
}

void VLog::Flush() {
    if (buffer.empty()) return;

    vlog_size += buffer.size();
    writer->Append(buffer);
    writer->Flush();
    buffer.clear();
    buffer.reserve(buffer_size_max * 2);
}

void VLog::Sync() {
    Flush();
    writer->Sync();
}


void VLog::Reset() {
    // std::lock_guard<std::mutex> lock(log_mutex);
    // Step 1: Clear the internal buffer
    buffer.clear();
    buffer.reserve(buffer_size_max * 2);
    // Step 2: Delete the existing file
    writer->Close();
    int status = remove(vlog_name.c_str());

    if (status != 0) {
        // Handle error in case file deletion fails
        std::cerr << "Error deleting file: " << vlog_name << std::endl;
        return;
    }
    // Step 3: Recreate the file
    adgMod::env->NewWritableFile(vlog_name, &writer);
    adgMod::env->NewRandomAccessFile(vlog_name, &reader); 

    memset(scratch, 0, sizeof(scratch));
    struct ::stat file_stat;
    ::stat(vlog_name.c_str(), &file_stat);
    vlog_size = file_stat.st_size;

}


VLog::~VLog() {
    Flush();
    delete[] scratch;
}
}