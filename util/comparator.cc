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

class VKeyComparatorImpl : public Comparator {
 public:
  VKeyComparatorImpl() {}

  virtual const char* Name() const { return "leveldb.VKeyComparator"; }

  virtual int Compare(const Slice& key1, const Slice& key2) const {
    int sz1 = key1.size(), sz2 = key2.size();
    assert(sz1 >= 16 && sz2 >= 16);
    const Slice skey1(key1.data() + sz1 - 8, 8);
    const Slice skey2(key2.data() + sz2 - 8, 8);
    int r = skey1.compare(skey2);
    if (r == 0) {
      const Slice ukey1(key1.data(), sz1 - 16);
      const Slice ukey2(key2.data(), sz2 - 16);
      r = ukey1.compare(ukey2);
      if (r == 0) {
        const uint64_t num1 = DecodeFixed64(key1.data() + sz1 - 16);
        const uint64_t num2 = DecodeFixed64(key2.data() + sz2 - 16);
        if (num1 > num2) {
          r = -1;
        } else if (num1 < num2) {
          r = +1;
        }
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
