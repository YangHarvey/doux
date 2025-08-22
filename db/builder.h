// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_BUILDER_H_
#define STORAGE_LEVELDB_DB_BUILDER_H_

#include "leveldb/status.h"
#include "util/arena.h"
#include "util/coding.h"
#include "impl/zorder/aabb.h"

namespace leveldb {

struct Options;
struct FileMetaData;

class Env;
class Iterator;
class TableCache;
class VersionEdit;

struct VInfo {
    Slice value;
    uint32_t file_number;
    uint32_t block_number;
    uint32_t block_offset;
};

static bool KCompare(const std::pair<Slice, VInfo>& p1, const std::pair<Slice, VInfo>& p2) {
  const Slice& s1 = p1.first;
  const Slice& s2 = p2.first;
  const size_t sz1 = s1.size(), sz2 = s2.size();
  const Slice ukey1(s1.data(), sz1 - 16);
  const Slice ukey2(s2.data(), sz2 - 16);
  int r = ukey1.compare(ukey2);
  if (r == 0) {
    const uint64_t num1 = DecodeFixed64(s1.data() + sz1 - 16);
    const uint64_t num2 = DecodeFixed64(s2.data() + sz2 - 16);
    if (num1 > num2) {
      r = -1;
    } else if (num1 < num2) {
      r = +1;
    }
  }
  return r < 0;
}

// Compare the key and value of the two slices
// return true if s1 is greater than s2
// return false if s1 is less than s2
static bool VKSliceCompare(const Slice& s1, const Slice& s2) {
  const size_t size_1 = s1.size(), size_2 = s2.size();
  // compare sort key
  uint64_t sort_key_1 = DecodeFixed64(s1.data() + size_1 - sizeof(uint64_t));
  uint64_t sort_key_2 = DecodeFixed64(s2.data() + size_2 - sizeof(uint64_t));
  int r = sort_key_1 > sort_key_2 ? -1 : (sort_key_1 < sort_key_2 ? 1 : 0);
  // if sort key is the same, compare key
  if (r == 0) {
    const Slice key_1(s1.data(), size_1 - 16);
    const Slice key_2(s2.data(), size_2 - 16);
    r = key_1.compare(key_2);
    if (r == 0) {
      const uint64_t num1 = DecodeFixed64(s1.data() + size_1 - 16);
      const uint64_t num2 = DecodeFixed64(s2.data() + size_2 - 16);
      if (num1 > num2) {
        r = 1;
      } else if (num1 < num2) {
        r = -1;
      }
    }
  }
  return r > 0;
}

static bool VCompare(const std::pair<Slice, VInfo>& p1, const std::pair<Slice, VInfo>& p2) {
  const Slice& s1 = p1.first;
  const Slice& s2 = p2.first;
  return VKSliceCompare(s1, s2);
}

static bool VKCompare(const std::pair<Slice, Slice>& p1, const std::pair<Slice, Slice>& p2) {
  const Slice& s1 = p1.first;
  const Slice& s2 = p2.first;
  return VKSliceCompare(s1, s2);
}

// Build a Table file from the contents of *iter.  The generated file
// will be named according to meta->number.  On success, the rest of
// *meta will be filled with metadata about the generated table.
// If no data is present in *iter, meta->file_size will be set to
// zero, and no Table file will be produced.
Status BuildTable(const std::string& dbname, Env* env, const Options& options,
                  TableCache* table_cache, Iterator* iter, FileMetaData* meta);
Status BuildDuTable(const std::string& dbname, Env* env, const Options& options,
                    TableCache* table_cache, Iterator* iter, FileMetaData* meta,
                    FileMetaData* vmeta, Arena* arena);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_BUILDER_H_
