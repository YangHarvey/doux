// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <algorithm>
#include <cstdint>
#include <string>

#include "leveldb/comparator.h"
#include "leveldb/slice.h"
#include "util/logging.h"
#include "util/no_destructor.h"

namespace leveldb {

void AppendInternalKey(std::string* result, const ParsedInternalKey& key) {
  result->append(key.user_key.data(), key.user_key.size());
  PutFixed64(result, PackSequenceAndType(key.sequence, key.type));
}

std::string ParsedInternalKey::DebugString() const {
  char buf[50];
  snprintf(buf, sizeof(buf), "' @ %llu : %d", (unsigned long long)sequence,
           int(type));
  std::string result = "'";
  result += EscapeString(user_key.ToString());
  result += buf;
  return result;
}

std::string InternalKey::DebugString() const {
  std::string result;
  ParsedInternalKey parsed;
  if (ParseInternalKey(rep_, &parsed)) {
    result = parsed.DebugString();
  } else {
    result = "(bad)";
    result.append(EscapeString(rep_));
  }
  return result;
}

Comparator::~Comparator() {}

namespace {
class BytewiseComparatorImpl : public Comparator {
 public:
  BytewiseComparatorImpl() {}

  virtual const char* Name() const { return "leveldb.BytewiseComparator"; }

  virtual int Compare(const Slice& a, const Slice& b) const {
    return a.compare(b);
  }

  virtual void FindShortestSeparator(std::string* start,
                                     const Slice& limit) const {
    // Find length of common prefix
    size_t min_length = std::min(start->size(), limit.size());
    size_t diff_index = 0;
    while ((diff_index < min_length) &&
           ((*start)[diff_index] == limit[diff_index])) {
      diff_index++;
    }

    if (diff_index >= min_length) {
      // Do not shorten if one string is a prefix of the other
    } else {
      uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
      if (diff_byte < static_cast<uint8_t>(0xff) &&
          diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
        (*start)[diff_index]++;
        start->resize(diff_index + 1);
        assert(Compare(*start, limit) < 0);
      }
    }
  }

  virtual void FindShortSuccessor(std::string* key) const {
    // Find first character that can be incremented
    size_t n = key->size();
    for (size_t i = 0; i < n; i++) {
      const uint8_t byte = (*key)[i];
      if (byte != static_cast<uint8_t>(0xff)) {
        (*key)[i] = byte + 1;
        key->resize(i + 1);
        return;
      }
    }
    // *key is a run of 0xffs.  Leave it alone.
  }
};
}  // namespace

const Comparator* BytewiseComparator() {
  static NoDestructor<BytewiseComparatorImpl> singleton;
  return singleton.get();
}

// ukey + sequence number + sort key
class VKeyComparatorImpl : public Comparator {
 public:
  VKeyComparatorImpl() {}

  virtual const char* Name() const { return "leveldb.VKeyComparator"; }

  // return true if key1 is greater than key2
  // return false if key1 is less than key2

  // vkey = ukey + sequence number + sort key
  virtual int Compare(const Slice& vkey1, const Slice& vkey2) const {
    int size_1 = vkey1.size(), size_2 = vkey2.size();
    assert(size_1 >= 16 && size_2 >= 16);
     
    // 1. compare sort key
    uint64_t sort_key_1 = DecodeFixed64(vkey1.data() + vkey1.size() - sizeof(uint64_t));
    uint64_t sort_key_2 = DecodeFixed64(vkey2.data() + vkey2.size() - sizeof(uint64_t));
    int r = sort_key_1 > sort_key_2 ? 1 : (sort_key_1 < sort_key_2 ? -1 : 0);

    // 2. if sort key is the same, compare key
    if (r == 0) {
      const Slice ukey1(vkey1.data(), size_1 - 16);
      const Slice ukey2(vkey2.data(), size_2 - 16);
      r = ukey1.compare(ukey2);
      if (r == 0) {
        // 3. if key is the same, compare sequence number
        const uint64_t seq1 = DecodeFixed64(vkey1.data() + size_1 - 16);
        const uint64_t seq2 = DecodeFixed64(vkey2.data() + size_2 - 16);
        r = seq1 > seq2 ? 1 : (seq1 < seq2 ? -1 : 0);
      }
    }
    return r;
  }

  virtual void FindShortestSeparator(std::string* start, const Slice& limit) const { return; }

  virtual void FindShortSuccessor(std::string* key) const { return; }
};

const Comparator* VKeyComparator() {
  static NoDestructor<VKeyComparatorImpl> singleton;
  return singleton.get();
}

}  // namespace leveldb
