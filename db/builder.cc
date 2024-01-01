// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <iostream>
#include "db/builder.h"

#include "db/dbformat.h"
#include "db/filename.h"
#include "db/table_cache.h"
#include "db/version_edit.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/iterator.h"

namespace leveldb {

Status BuildTable(const std::string& dbname, Env* env, const Options& options,
                  TableCache* table_cache, Iterator* iter, FileMetaData* meta) {
  Status s;
  meta->file_size = 0;
  iter->SeekToFirst();

  std::string fname = TableFileName(dbname, meta->number);
  if (iter->Valid()) {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    TableBuilder* builder = new TableBuilder(options, file);
    meta->smallest.DecodeFrom(iter->key());
    for (; iter->Valid(); iter->Next()) {
      Slice key = iter->key();
      meta->largest.DecodeFrom(key);
      builder->Add(key, iter->value());
    }

    // Finish and check for builder errors
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      assert(meta->file_size > 0);
    }
    delete builder;

    // Finish and check for file errors
    if (s.ok()) {
      s = file->Sync();
    }
    if (s.ok()) {
      s = file->Close();
    }
    delete file;
    file = nullptr;

    if (s.ok()) {
      // Verify that the table is usable
      Iterator* it = table_cache->NewIterator(ReadOptions(), meta->number,
                                              meta->file_size);
      s = it->status();
      delete it;
    }
  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(fname);
  }
  return s;
}

Slice ConstructVKey(const Slice& key, const Slice& value, Arena* arena) {
  char* buf = arena->Allocate(key.size() + sizeof(uint64_t));
  char* p = buf;
  size_t key_size = key.size();
  memcpy(p, key.data(), key_size);
  p += key_size;

  uint32_t val1 = DecodeFixed32(value.data());
  uint32_t val2 = DecodeFixed32(value.data() + sizeof(uint32_t));
  if (adgMod::MOD == 9) {
    uint64_t sort_key = val1;
    sort_key = (sort_key << 32) | val2;
    EncodeFixed64(p, sort_key);
  } else if (adgMod::MOD == 10) {
    doux::MortonCode<2, 32> sort_key(0);
    sort_key.Encode({val1, val2});
    EncodeFixed64(p, sort_key.data_);
  }

  return Slice(buf, key_size + sizeof(uint64_t));
}

Status BuildDuTable(const std::string& dbname, Env* env, const Options& options,
                    TableCache* table_cache, Iterator* iter, FileMetaData* meta,
                    Iterator* viter, FileMetaData* vmeta, Arena* arena) {
  Status s;
  meta->file_size = 0;
  iter->SeekToFirst();
  vmeta->file_size = 0;
  viter->SeekToFirst();

  std::string fname = TableFileName(dbname, meta->number);
  std::string vfname = VTableFileName(dbname, vmeta->number);

  // Generate value table
  if (viter->Valid()) {
    WritableFile* vfile;
    s = env->NewWritableFile(vfname, &vfile);
    if (!s.ok()) {
      return s;
    }

    vector<std::pair<Slice, VInfo>> kvs;
    for (; viter->Valid(); viter->Next()) {
      Slice key = viter->key();
      Slice value = viter->value();
      Slice vkey = ConstructVKey(key, value, arena);
      VInfo info;
      info.value = value;
      kvs.emplace_back(vkey, info);
    }

    auto cmp = [](const std::pair<Slice, VInfo>& p1, const std::pair<Slice, VInfo>& p2) {
      const Slice& s1 = p1.first;
      const Slice& s2 = p2.first;
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
    };

    std::sort(kvs.begin(), kvs.end(), cmp);

    TableBuilder* builder = new TableBuilder(options, vfile);
    vmeta->smallest.DecodeFrom(kvs[0].first);
    vmeta->largest.DecodeFrom(kvs[kvs.size() - 1].first);
    for (const auto& kv : kvs) {
      builder->Add(kv.first, kv.second.value);
    }

    // Finish and check for builder errors
    s = builder->Finish();
    if (s.ok()) {
      vmeta->file_size = builder->FileSize();
      assert(vmeta->file_size > 0);
    }
    delete builder;

    // Finish and check for file errors
    if (s.ok()) {
      s = vfile->Sync();
    }
    if (s.ok()) {
      s = vfile->Close();
    }
    delete vfile;
    vfile = nullptr;

    // if (s.ok()) {
    //   // Verify that the table is usable
    //   Iterator* it = table_cache->NewIterator(ReadOptions(), vmeta->number,
    //                                           vmeta->file_size);
    //   s = it->status();
    //   delete it;
    // }
  }

  // Check for input iterator errors
  if (!viter->status().ok()) {
    s = viter->status();
  }

  if (s.ok() && vmeta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(vfname);
  }


  // Generate key table
  if (iter->Valid()) {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    TableBuilder* builder = new TableBuilder(options, file);
    meta->smallest.DecodeFrom(iter->key());
    for (; iter->Valid(); iter->Next()) {
      Slice key = iter->key();
      meta->largest.DecodeFrom(key);
      uint32_t file_number = static_cast<uint32_t>(vmeta->number);
      uint32_t file_size = static_cast<uint32_t>(vmeta->file_size);
      char buffer[sizeof(uint32_t) * 4];
      EncodeFixed32(buffer, file_number);
      EncodeFixed32(buffer + sizeof(uint32_t), file_size);
      EncodeFixed32(buffer + sizeof(uint32_t) * 2, builder->BlockNumber());
      EncodeFixed32(buffer + sizeof(uint32_t) * 3, builder->BlockOffset());
      builder->Add(key, (Slice) {buffer, sizeof(uint32_t) * 4});
    }

    // Finish and check for builder errors
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      assert(meta->file_size > 0);
    }
    delete builder;

    // Finish and check for file errors
    if (s.ok()) {
      s = file->Sync();
    }
    if (s.ok()) {
      s = file->Close();
    }
    delete file;
    file = nullptr;

    if (s.ok()) {
      // Verify that the table is usable
      Iterator* it = table_cache->NewIterator(ReadOptions(), meta->number,
                                              meta->file_size);
      s = it->status();
      delete it;
    }
  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(fname);
  }
  
  return s;
}

}  // namespace leveldb
