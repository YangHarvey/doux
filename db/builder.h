// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_DB_BUILDER_H_
#define STORAGE_LEVELDB_DB_BUILDER_H_

#include "leveldb/status.h"
#include "util/arena.h"
#include "impl/zorder/encoding.h"

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
    uint32_t file_size;
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

static bool VKSliceCompare(const Slice& s1, const Slice& s2) {
  const size_t sz1 = s1.size(), sz2 = s2.size();
  const Slice skey1(s1.data() + sz1 - 8, 8);
  const Slice skey2(s2.data() + sz2 - 8, 8);
  int r = skey1.compare(skey2);
  if (r == 0) {
    const Slice ukey1(s1.data(), sz1 - 16);
    const Slice ukey2(s2.data(), sz2 - 16);
    r = ukey1.compare(ukey2);
    if (r == 0) {
      const uint64_t num1 = DecodeFixed64(s1.data() + sz1 - 16);
      const uint64_t num2 = DecodeFixed64(s2.data() + sz2 - 16);
      if (num1 > num2) {
        r = -1;
      } else if (num1 < num2) {
        r = +1;
      }
    }
  }
  return r < 0;
}

static bool VCompare(const std::pair<Slice, VInfo>& p1, const std::pair<Slice, VInfo>& p2) {
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
                    Iterator* viter, FileMetaData* vmeta, Arena* arena);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_DB_BUILDER_H_
