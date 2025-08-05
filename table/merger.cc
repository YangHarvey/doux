// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "table/merger.h"


#include "leveldb/comparator.h"
#include "leveldb/iterator.h"
#include "table/iterator_wrapper.h"
#include "util/coding.h"
#include "mod/util.h"
#include "impl/zorder/aabb.h"

namespace leveldb {

namespace {

// Which direction is the iterator moving?
enum Direction { kForward, kReverse };

class MergingIterator : public Iterator {
 public:
  MergingIterator(const Comparator* comparator, Iterator** children, int n)
      : comparator_(comparator),
        children_(new IteratorWrapper[n]),
        n_(n),
        current_(nullptr),
        direction_(kForward) {
    for (int i = 0; i < n; i++) {
      children_[i].Set(children[i]);
    }
  }

  virtual ~MergingIterator() { delete[] children_; }

  virtual bool Valid() const { return (current_ != nullptr); }

  virtual void SeekToFirst() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToFirst();
    }
    FindSmallest();
    direction_ = kForward;
  }

  virtual void SeekToLast() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToLast();
    }
    FindLargest();
    direction_ = kReverse;
  }

  virtual void Seek(const Slice& target) {
    for (int i = 0; i < n_; i++) {
      children_[i].Seek(target);
    }
    FindSmallest();
    direction_ = kForward;
  }

  virtual void Next() {
    assert(Valid());

    // Ensure that all children are positioned after key().
    // If we are moving in the forward direction, it is already
    // true for all of the non-current_ children since current_ is
    // the smallest child and key() == current_->key().  Otherwise,
    // we explicitly position the non-current_ children.
    if (direction_ != kForward) {
      for (int i = 0; i < n_; i++) {
        IteratorWrapper* child = &children_[i];
        if (child != current_) {
          child->Seek(key());
          if (child->Valid() &&
              comparator_->Compare(key(), child->key()) == 0) {
            child->Next();
          }
        }
      }
      direction_ = kForward;
    }

    current_->Next();
    FindSmallest();
  }

  virtual void Prev() {
    assert(Valid());

    // Ensure that all children are positioned before key().
    // If we are moving in the reverse direction, it is already
    // true for all of the non-current_ children since current_ is
    // the largest child and key() == current_->key().  Otherwise,
    // we explicitly position the non-current_ children.
    if (direction_ != kReverse) {
      for (int i = 0; i < n_; i++) {
        IteratorWrapper* child = &children_[i];
        if (child != current_) {
          child->Seek(key());
          if (child->Valid()) {
            // Child is at first entry >= key().  Step back one to be < key()
            child->Prev();
          } else {
            // Child has no entries >= key().  Position at last entry.
            child->SeekToLast();
          }
        }
      }
      direction_ = kReverse;
    }

    current_->Prev();
    FindLargest();
  }

  virtual Slice key() const {
    assert(Valid());
    return current_->key();
  }

  virtual Slice value() const {
    assert(Valid());
    return current_->value();
  }

  virtual Status status() const {
    Status status;
    for (int i = 0; i < n_; i++) {
      status = children_[i].status();
      if (!status.ok()) {
        break;
      }
    }
    return status;
  }

 private:

  void FindSmallest();
  void FindLargest();

  // We might want to use a heap in case there are lots of children.
  // For now we use a simple array since we expect a very small number
  // of children in leveldb.
  const Comparator* comparator_;
  IteratorWrapper* children_;
  int n_;
  IteratorWrapper* current_;
  Direction direction_;
};

void MergingIterator::FindSmallest() {
  IteratorWrapper* smallest = nullptr;
  for (int i = 0; i < n_; i++) {
    IteratorWrapper* child = &children_[i];
    if (child->Valid()) {
      if (smallest == nullptr) {
        smallest = child;
      } else if (comparator_->Compare(child->key(), smallest->key()) < 0) {
        smallest = child;
      }
    }
  }
  current_ = smallest;
}

void MergingIterator::FindLargest() {
  IteratorWrapper* largest = nullptr;
  for (int i = n_ - 1; i >= 0; i--) {
    IteratorWrapper* child = &children_[i];
    if (child->Valid()) {
      if (largest == nullptr) {
        largest = child;
      } else if (comparator_->Compare(child->key(), largest->key()) > 0) {
        largest = child;
      }
    }
  }
  current_ = largest;
}

class VMergingIterator : public Iterator {
 public:
  VMergingIterator(const ReadOptions& options, const Comparator* comparator, Iterator** children, int n) 
      : comparator_(comparator),
        children_(new IteratorWrapper[n]),
        n_(n),
        current_(nullptr),
        direction_(kForward),
        pos_(0) {
    for (int i = 0; i < n; i++) {
      children_[i].Set(children[i]);
    }

    start_ = options.start1;
    start_ = (start_ << 32) | options.start2;
    end_ = options.end1;
    end_ = (end_ << 32) | options.end2;

    big_min_ = doux::MortonCode<2, 32>::Encode({options.start1, options.start2});
    limit_max_ = doux::MortonCode<2, 32>::Encode({options.end1, options.end2});
    doux::AABB<2, 32> aabb = {big_min_, limit_max_};
    region_ = aabb.ToIntervals();
    cur_vkey_ = new char[adgMod::key_size + 16];
    cur_idx_.reserve(n);
    for (int i = 0; i < n; i++) {
      cur_idx_[i] = 0;
    }
  }

  virtual ~VMergingIterator() { 
    delete[] children_;
    delete[] cur_vkey_;
  }

  virtual bool Valid() const { return (current_ != nullptr); }

  virtual void SeekToFirst() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToFirst();
    }
    current_ = &children_[0];
    direction_ = kForward;
  }

  virtual void SeekToLast() {
    for (int i = 0; i < n_; i++) {
      children_[i].SeekToLast();
    }
    current_ = &children_[n_ - 1];
    direction_ = kReverse;
  }

  virtual void Seek(const Slice& target) {
    if (adgMod::MOD == 9) {
      EncodeFixed64(cur_vkey_, 0);
      EncodeFixed64(cur_vkey_ + 8, 0);
      EncodeFixed64(cur_vkey_ + 16, PackSequenceAndType(kMaxSequenceNumber, kValueTypeForSeek));
      EncodeFixed32(cur_vkey_ + 24, options_.start1);
      EncodeFixed32(cur_vkey_ + 28, options_.start2);
      Slice vkey(cur_vkey_, adgMod::key_size + 16);
      for (int i = 0; i < n_; i++) {
        children_[i].Seek(vkey);
        if (children_[i].Valid()) {
          children_[i].Next();
        }
      }
    } else if (adgMod::MOD == 10) {
      EncodeFixed64(cur_vkey_, 0);
      EncodeFixed64(cur_vkey_ + 8, 0);
      EncodeFixed64(cur_vkey_ + 16, PackSequenceAndType(kMaxSequenceNumber, kValueTypeForSeek));
      for (int i = 0; i < n_; i++) {
        EncodeFixed64(cur_vkey_ + 24, region_.intervals_[cur_idx_[i]].start_);
        Slice vkey(cur_vkey_, adgMod::key_size + 16);
        children_[i].Seek(vkey);
        if (children_[i].Valid()) {
          children_[i].Next();
        }
      }
    }

    for (int i = 0; i < n_; i++) {
      if (children_[i].Valid()) {
        current_ = &children_[i];
        pos_ = i;
        break;
      }
    }
    direction_ = kForward;
  }

  virtual void Next() {
    assert(Valid());
    current_->Next();
    if (adgMod::MOD == 10) {
      if (!current_ || !current_->Valid()) {
        // 如果当前迭代器无效，尝试移动到下一个有效的迭代器
        while (pos_ < n_ && (!current_ || !current_->Valid())) {
          ++pos_;
          if (pos_ < n_) current_ = &children_[pos_];
        }
        // 如果所有迭代器都无效，将 current_ 设为 nullptr
        if (pos_ >= n_ || !current_ || !current_->Valid()) {
          current_ = nullptr;
          return;
        }
      }
      
      Slice cur_vkey = current_->key();
      uint64_t cur_zorder = DecodeFixed64(cur_vkey.data() + adgMod::key_size + 8);
      const auto& cur_interval = region_.intervals_[cur_idx_[pos_]];
      if (!cur_interval.Contains(doux::MortonCode<2, 32>(cur_zorder))) {
        while (pos_ < n_ && !JumpNext()) {
          ++pos_;
          current_ = &children_[pos_];
        }
        // 检查循环结束后 current_ 是否仍然有效
        if (pos_ >= n_ || !current_ || !current_->Valid()) {
          current_ = nullptr;
          return;
        }
      }
    } else if (adgMod::MOD == 9) {
      if (!current_ || !current_->Valid()) {
        // std::cout << "MOD 9: Current invalid, trying to switch to next child..." << std::endl;
        // 如果当前迭代器无效，尝试移动到下一个有效的迭代器
        while (pos_ < n_ && (!current_ || !current_->Valid())) {
          ++pos_;
          if (pos_ < n_) {
            current_ = &children_[pos_];
            // std::cout << "MOD 9 switched to child " << pos_ << ": iter=" << (current_->iter() ? "valid" : "nullptr")
                      // << ", wrapper_valid=" << current_->Valid() << std::endl;
          }
        }
        // 如果所有迭代器都无效，将 current_ 设为 nullptr
        if (pos_ >= n_ || !current_ || !current_->Valid()) {
          std::cout << "MOD 9: All children invalid, setting current_ to nullptr" << std::endl;
          current_ = nullptr;
          return;
        }
        // 如果成功切换到有效迭代器，继续处理
        // std::cout << "MOD 9: Successfully switched to valid child " << pos_ << std::endl;
      }
      
      Slice cur_vkey = current_->key();
      // 检查 cur_vkey 是否足够大
      if (cur_vkey.size() < adgMod::key_size + 16) {
        // 直接跳过或报错
        while (pos_ < n_ && (!current_ || !current_->Valid())) {
          ++pos_;
          if (pos_ < n_) {
            current_ = &children_[pos_];
            // std::cout << "MOD 9 switched to child " << pos_ << ": iter=" << (current_->iter() ? "valid" : "nullptr")
                      // << ", wrapper_valid=" << current_->Valid() << std::endl;
            // 确保新切换的迭代器是有效的
            if (!current_->Valid()) {
              // std::cout << "MOD 9: Child " << pos_ << " is invalid, setting current_ to nullptr" << std::endl;
              current_ = nullptr;
            }
          }
        }
        // 检查循环结束后 current_ 是否仍然有效
        if (pos_ >= n_ || !current_ || !current_->Valid()) {
          std::cout << "MOD 9: All children invalid, setting current_ to nullptr" << std::endl;
          current_ = nullptr;
          return;
        }
        return;
      }
      uint64_t cur_sort_key = DecodeFixed64(cur_vkey.data() + adgMod::key_size + 8);
      if (cur_sort_key < start_ || cur_sort_key > end_) {
        // std::cout << "MOD 9: Sort key out of range (" << cur_sort_key << " not in [" << start_ << ", " << end_ << "]), switching..." << std::endl;
        while (pos_ < n_ && (!current_ || !current_->Valid())) {
          ++pos_;
          if (pos_ < n_) {
            current_ = &children_[pos_];
            // std::cout << "MOD 9 range switched to child " << pos_ << ": iter=" << (current_->iter() ? "valid" : "nullptr")
                      // << ", wrapper_valid=" << current_->Valid() << std::endl;
            // 确保新切换的迭代器是有效的
            if (!current_->Valid()) {
              // std::cout << "MOD 9 range: Child " << pos_ << " is invalid, setting current_ to nullptr" << std::endl;
              current_ = nullptr;
            }
          }
        }
        // 检查循环结束后 current_ 是否仍然有效
        if (pos_ >= n_ || !current_ || !current_->Valid()) {
          // std::cout << "MOD 9 range: All children invalid, setting current_ to nullptr" << std::endl;
          current_ = nullptr;
          return;
        }
      }
    }
  }

  virtual void Prev() {
    assert(Valid());
    current_->Prev();
    if (pos_ > 0 && !current_->Valid()) {
      --pos_;
      current_ = &children_[pos_];
      // 确保新切换的迭代器是有效的
      if (!current_->Valid()) {
        current_ = nullptr;
      }
    }
    // 检查 current_ 是否仍然有效
    if (!current_ || !current_->Valid()) {
      current_ = nullptr;
    }
  }

  virtual Slice key() const {
    assert(Valid());
    return current_->key();
  }

  virtual Slice value() const {
    assert(Valid());
    return current_->value();
  }

  virtual Status status() const {
    Status status;
    for (int i = 0; i < n_; i++) {
      status = children_[i].status();
      if (!status.ok()) {
        break;
      }
    }
    return status;
  }

 private:
  bool JumpNext() {
    EncodeFixed64(cur_vkey_, 0);
    EncodeFixed64(cur_vkey_ + 8, 0);
    EncodeFixed64(cur_vkey_ + 16, PackSequenceAndType(kMaxSequenceNumber, kValueTypeForSeek));
    cur_idx_[pos_]++;
    while (cur_idx_[pos_] < region_.intervals_.size()) {
      const auto& cur_interval = region_.intervals_[cur_idx_[pos_]];
      EncodeFixed64(cur_vkey_ + 24, region_.intervals_[cur_idx_[pos_]].start_);
      current_->Seek({cur_vkey_, static_cast<size_t>(adgMod::key_size + 16)});
      if (current_->Valid()) {
        current_->Next();
        break;
      }
    }
    if (!current_->Valid()) {
      return false;
    }
    return true;
  }

 private:
  ReadOptions options_;
  const Comparator* comparator_;
  IteratorWrapper* children_;
  int n_;
  IteratorWrapper* current_;
  Direction direction_;
  
  int pos_;
  uint64_t start_;
  uint64_t end_;
  doux::MortonCode<2, 32> big_min_;
  doux::MortonCode<2, 32> limit_max_;
  doux::Region<2, 32> region_;
  char* cur_vkey_;
  std::vector<int> cur_idx_;
};

}  // namespace

Iterator* NewMergingIterator(const Comparator* comparator, Iterator** children,
                             int n) {
  assert(n >= 0);
  if (n == 0) {
    return NewEmptyIterator();
  } else if (n == 1) {
    return children[0];
  } else {
    return new MergingIterator(comparator, children, n);
  }
}

Iterator* NewVMergingIterator(const ReadOptions& options, const Comparator* comparator,
                              Iterator** children, int n) {
  assert(n >= 0);
  if (n == 0) {
    return NewEmptyIterator();
  } else if (n == 1) {
    return children[0];
  } else {
    return new VMergingIterator(options, comparator, children, n);
  }
}

}  // namespace leveldb
