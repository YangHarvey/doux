// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

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

Status BuildDuTable(const std::string& dbname, Env* env, const Options& options,
                    TableCache* table_cache, Iterator* iter, FileMetaData* meta,
                    Iterator* viter, FileMetaData* vmeta) {
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

    TableBuilder* builder = new TableBuilder(options, vfile);
    vmeta->smallest.DecodeFrom(viter->key());
    for (; viter->Valid(); viter->Next()) {
      Slice key = viter->key();
      vmeta->largest.DecodeFrom(key);
      builder->Add(key, viter->value());
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
      char buffer[sizeof(uint64_t) + sizeof(uint64_t)];
      EncodeFixed64(buffer, vmeta->number);
      EncodeFixed64(buffer + sizeof(uint64_t), vmeta->file_size);
      builder->Add(key, (Slice) {buffer, sizeof(uint64_t) + sizeof(uint64_t)});
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
