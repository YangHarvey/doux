#include <fcntl.h>
#include <sys/stat.h>
#include "Vlog.h"
#include "util.h"
#include "util/coding.h"

using std::string;



const int buffer_size_max = 300 * 1024;

namespace adgMod {

VLog::VLog(const std::string& vlog_name) : writer(nullptr), reader(nullptr) {
    adgMod::env->NewWritableFile(vlog_name, &writer);
    adgMod::env->NewRandomAccessFile(vlog_name, &reader);
    adgMod::env->NewWritableFile(vlog_name + ".latest", &writer1);
    adgMod::env->NewRandomAccessFile(vlog_name + ".latest", &reader1);
    buffer.reserve(buffer_size_max * 2);
    buffer1.reserve(1024 * 1024);
    struct ::stat file_stat;
    ::stat(vlog_name.c_str(), &file_stat);
    vlog_size = file_stat.st_size;
}

uint64_t VLog::AddRecord(const Slice& key, const Slice& value) {
    PutLengthPrefixedSlice(&buffer, key);
    PutVarint32(&buffer, value.size());
    uint64_t result = vlog_size + buffer.size();
    buffer.append(value.data(), value.size());

    if (buffer.size() >= buffer_size_max) Flush();
    return result;
}

string VLog::ReadRecord(uint64_t address, uint32_t size) {
    if (address >= vlog_size) return string(buffer.c_str() + address - vlog_size, size);

    char* scratch = new char[size];
    Slice value;
    reader->Read(address, size, &value, scratch);
    string result(value.data(), value.size());
    delete[] scratch;
    return result;
}

Slice VLog::ReadRecord2(uint64_t address, uint32_t size) {
    if (address >= vlog_size) return string(buffer.c_str() + address - vlog_size, size);

    static char scratch[4096];
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

void VLog::GC() {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 1024; j++) {
            string value(1024, '0');
            buffer1.append(value.data(), value.size());
        }
        writer1->Append(buffer1);
        writer1->Flush();
        buffer1.clear();
        buffer1.reserve(1024 * 1024);
        writer->Sync();
    }
}

VLog::~VLog() {
    Flush();
}































}