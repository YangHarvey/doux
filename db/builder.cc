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
  
   vector<std::pair<Slice, VInfo>> kvs;
  // Generate value table
  if (viter->Valid()) {
    WritableFile* vfile;
    s = env->NewWritableFile(vfname, &vfile);
    if (!s.ok()) {
      return s;
    }

    for (; viter->Valid(); viter->Next()) {
      Slice key = viter->key();
      Slice value = viter->value();
      Slice vkey = ConstructVKey(key, value, arena);
      VInfo info;
      info.value = value;
      kvs.emplace_back(vkey, info);
    }

    std::sort(kvs.begin(), kvs.end(), VCompare);

    TableBuilder* builder = new TableBuilder(options, vfile);
    Slice min_key = kvs[0].first;
    Slice max_key = kvs[kvs.size() - 1].first;
    if (adgMod::MOD == 9) {
      min_key.remove_suffix(8);
      max_key.remove_suffix(8);
    }
    vmeta->smallest.DecodeFrom(min_key);
    vmeta->largest.DecodeFrom(max_key);
    for (auto& kv : kvs) {
      kv.second.file_number = static_cast<uint32_t>(vmeta->number);
      kv.second.block_number = builder->BlockNumber();
      kv.second.block_offset = builder->BlockOffset();
      builder->Add(kv.first, kv.second.value);
    }

    // Finish and check for builder errors
    s = builder->Finish();
    if (s.ok()) {
      vmeta->file_size = builder->FileSize();
      for (auto& kv : kvs) {
        kv.second.file_size = static_cast<uint32_t>(vmeta->file_size);
      }
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

    // std::cout << "Write VSST " << vfname << " to L0 successfully!" << std::endl;

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

    std::sort(kvs.begin(), kvs.end(), KCompare);

    TableBuilder* builder = new TableBuilder(options, file);
    meta->smallest.DecodeFrom(iter->key());
    int idx = 0;
    for (; iter->Valid(); iter->Next()) {
      Slice key = iter->key();
      meta->largest.DecodeFrom(key);
      char buffer[sizeof(uint32_t) * 4];
      EncodeFixed32(buffer, kvs[idx].second.file_number);
      EncodeFixed32(buffer + sizeof(uint32_t), kvs[idx].second.file_size);
      EncodeFixed32(buffer + sizeof(uint32_t) * 2, kvs[idx].second.block_number);
      EncodeFixed32(buffer + sizeof(uint32_t) * 3, kvs[idx].second.block_offset);
      builder->Add(key, (Slice) {buffer, sizeof(uint32_t) * 4});
      ++idx;
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
