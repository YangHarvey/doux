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
      meta->num_keys = builder->NumEntries();
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

// Construct the key for the value table
// Vkey: [key, sort key]
// sort key = Morton code of [value1, value2]
Slice ConstructVKey(const Slice& key, const Slice& value, Arena* arena) {
  // 分配内存
  size_t key_size = key.size();
  size_t sort_key_size = sizeof(uint64_t);
  char* buf = arena->Allocate(key_size + sort_key_size);

  // 复制key
  char* p = buf;
  memcpy(p, key.data(), key_size);
  p += key_size;

  // 计算sort key
  uint32_t val1 = DecodeBigEndianFixed32(value.data());
  uint32_t val2 = DecodeBigEndianFixed32(value.data() + sizeof(uint32_t));
  if (adgMod::MOD == 9) {
    uint64_t sort_key = val1;
    sort_key = (sort_key << 32) | val2;
    EncodeFixed64(p, sort_key);
  } else if (adgMod::MOD == 10 || adgMod::MOD == 13) {
    auto sort_key = doux::MortonCode<2, 32>::Encode({val1, val2});
    EncodeFixed64(p, sort_key);
  }

  return Slice(buf, key_size + sizeof(uint64_t));
}

void ParseVKey(const Slice& vkey) {
  assert(vkey.size() > sizeof(uint64_t));
  // sort key
  Slice sort_key = Slice(vkey.data() + vkey.size() - sizeof(uint64_t), sizeof(uint64_t));
  uint64_t sort_key_value = DecodeFixed64(sort_key.data());
  // key
  Slice key = Slice(vkey.data(), vkey.size() - sizeof(uint64_t));
  std::cout << "sort_key_value: " << sort_key_value << ", key: " << key.ToString() << std::endl;
}

inline Slice ConstructSlice(const Slice& from, Arena* arena) {
  size_t size = from.size();
  char* buf = arena->Allocate(size);
  memcpy(buf, from.data(), size);
  return Slice(buf, size);
}

Status BuildDuTable(const std::string& dbname, Env* env, const Options& options,
                    TableCache* table_cache, Iterator* iter, FileMetaData* meta,
                    FileMetaData* vmeta, Arena* arena) {
  Status s;
  iter->SeekToFirst();
  meta->file_size = 0;
  vmeta->file_size = 0;

  std::string fname = TableFileName(dbname, meta->number);
  std::string vfname = VTableFileName(dbname, vmeta->number);
  
  // Generate value table
  vector<std::pair<Slice, VInfo>> kvs;
  if (iter->Valid()) {
    WritableFile* vfile;
    s = env->NewWritableFile(vfname, &vfile);
    if (!s.ok()) {
      return s;
    }

    for (; iter->Valid(); iter->Next()) {
      Slice key = iter->key();
      Slice value = iter->value();
      Slice tmpKey = adgMod::MOD == 10 ? ConstructVKey(key, value, arena) : ConstructSlice(key, arena);
      VInfo info;
      info.value = value;
      kvs.emplace_back(tmpKey, info);
    }

    // 获取value table的min_key和max_key
    Slice min_key, max_key;
    if (adgMod::MOD == 10 || adgMod::MOD == 13) {
      std::sort(kvs.begin(), kvs.end(), VCompare);
      // 排序后，min_key和max_key直接取首尾
      min_key = kvs[0].first;
      max_key = kvs[kvs.size() - 1].first;
      
      // 后续代码使用min_key和max_key
    } else if(adgMod::MOD == 9) {
      // 没有排序，min_key和max_key需要遍历得到
      min_key = kvs[0].first;
      max_key = kvs[0].first;
      for (size_t i = 1; i < kvs.size(); ++i) {
        if (kvs[i].first.compare(min_key) < 0) min_key = kvs[i].first;
        if (kvs[i].first.compare(max_key) > 0) max_key = kvs[i].first;
      }
    }

    vmeta->smallest.DecodeFrom(min_key);
    vmeta->largest.DecodeFrom(max_key);

    TableBuilder* vbuilder;
    vbuilder = new TableBuilder(options, vfile);
    for (auto& kv : kvs) {
      kv.second.file_number = static_cast<uint32_t>(vmeta->number);
      kv.second.block_number = vbuilder->BlockNumber();
      kv.second.block_offset = vbuilder->BlockOffset();
      vbuilder->Add(kv.first, kv.second.value);
    }

    // Finish and check for builder errors
    s = vbuilder->Finish();
    if (s.ok()) {
      vmeta->file_size = vbuilder->FileSize();
      vmeta->num_keys = vbuilder->NumEntries();
      assert(vmeta->file_size > 0);
    }
    delete vbuilder;

    // Finish and check for file errors
    if (s.ok()) {
      s = vfile->Sync();
    }
    if (s.ok()) {
      s = vfile->Close();
    }
    delete vfile;
    vfile = nullptr;

  }

  // Check for input iterator errors
  if (!iter->status().ok()) {
    s = iter->status();
  }

  if (s.ok() && vmeta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(vfname);
  }

  // Generate key table
  {
    WritableFile* file;
    s = env->NewWritableFile(fname, &file);
    if (!s.ok()) {
      return s;
    }

    Slice min_key, max_key;
    if (adgMod::MOD == 10 || adgMod::MOD == 13) {
      std::sort(kvs.begin(), kvs.end(), KCompare);
      min_key = kvs[0].first;
      max_key = kvs[kvs.size() - 1].first;
    } else if(adgMod::MOD == 9) {
      min_key = kvs[0].first;
      max_key = kvs[0].first;
      for (const auto& kv : kvs) {
        if (kv.first.compare(min_key) < 0) min_key = kv.first;
        if (kv.first.compare(max_key) > 0) max_key = kv.first;
      }
    }
    if (adgMod::MOD == 9 || adgMod::MOD == 10 || adgMod::MOD == 13) {
      min_key.remove_suffix(8);
      max_key.remove_suffix(8);
    }

    meta->smallest.DecodeFrom(min_key);
    meta->largest.DecodeFrom(max_key);

    TableBuilder* builder = new TableBuilder(options, file);
    for (const auto& kv : kvs) {
      Slice key = kv.first;
      char buffer[sizeof(uint32_t) * 3];
      EncodeFixed32(buffer, kv.second.file_number);
      EncodeFixed32(buffer + sizeof(uint32_t), kv.second.block_number);
      EncodeFixed32(buffer + sizeof(uint32_t) * 2, kv.second.block_offset);
      if (adgMod::MOD == 9 || adgMod::MOD == 10 || adgMod::MOD == 13) {
        key.remove_suffix(8);
      }
      builder->Add(key, (Slice) {buffer, sizeof(uint32_t) * 3});
    }

    // Finish and check for builder errors
    s = builder->Finish();
    if (s.ok()) {
      meta->file_size = builder->FileSize();
      meta->num_keys = builder->NumEntries();
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

  if (s.ok() && meta->file_size > 0) {
    // Keep it
  } else {
    env->DeleteFile(fname);
  }
  
  return s;
}

}  // namespace leveldb
