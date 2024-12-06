#ifndef ZORDER_INTERVAL_H
#define ZORDER_INTERVAL_H

#include <cstdint>
#include <cassert>

#include <vector>
#include <tuple>
#include <optional>

#include "impl/zorder/encoding.h"
#include "impl/zorder/util.h"
#include <immintrin.h>

namespace doux {

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T = std::monostate>
struct Interval {
    MortonCode<2, 32> start_, end_;
    T data_ {};
    Interval(MortonCode<2, 32> start, MortonCode<2, 32> end): start_(start), end_(end) {};
    Interval(MortonCode<2, 32> start, MortonCode<2, 32> end, T t): start_(start), end_(end), data_(t) {};

    template<typename M>
    friend bool operator==(const Interval& lhs, const Interval<Dimension, BitsPerDimension, M>& rhs) {
        if constexpr (std::is_same<T, std::monostate>::value || std::is_same<M, std::monostate>::value) {
            return std::tie(lhs.start_, lhs.end_) == std::tie(rhs.start_, rhs.end_);
        } else if constexpr (std::is_same<T, M>::value) {
            return std::tie(lhs.start_, lhs.end_, lhs.data_) == std::tie(rhs.start_, rhs.end_, rhs.data_);
        } else {
            static_assert(std::is_same<T, std::monostate>::value || std::is_same<M, std::monostate>::value || std::is_same<T, M>::value, "the data_ types on intervals should be either the same or std::monostate");
        }
    }

    friend bool operator!=(const Interval& lhs, const Interval& rhs) {
        return !(lhs == rhs);
    }

    template<typename M>
    friend bool operator<(const Interval& lhs, const Interval<Dimension, BitsPerDimension, M>& rhs) {
        if constexpr (std::is_same<T, std::monostate>::value || std::is_same<M, std::monostate>::value) {
            return std::tie(lhs.start_, lhs.end_) < std::tie(rhs.start_, rhs.end_);
        } else if constexpr (std::is_same<T, M>::value) {
            return std::tie(lhs.start_, lhs.end_, lhs.data_) < std::tie(rhs.start_, rhs.end_, rhs.data_);
        } else {
            static_assert(std::is_same<T, std::monostate>::value || std::is_same<M, std::monostate>::value || std::is_same<T, M>::value, "the data_ types on intervals should be either the same or std::monostate");
        }
    }

    friend bool operator>(const Interval& lhs, const Interval& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const Interval& lhs, const Interval& rhs) {
        return !(lhs > rhs);
    }

    friend bool operator>=(const Interval& lhs, const Interval& rhs) {
        return !(lhs < rhs);
    }

    bool DataEquals(const Interval& rhs) const;

     std::optional<Interval> Intersect(const Interval& rhs) const;

    bool Contains(const MortonCode<Dimension, BitsPerDimension> c) const {
        return c >= start_ && c <= end_;
    }

    uint64_t Area() const;

    uint64_t StartAlignment() const;

    uint64_t EndAlignment() const;

    std::vector<Interval> ToCells() const;

    std::vector<Interval> ToCells(size_t max_level) const;
    // this returns an sorted map of the cells and size.
    // e.g. 3 cells of size 1, 2 cells of size 2, and one cell of size 3.
    // where size 1 = 1 on each side, size 2 = 2, size 3  = 4
    std::vector<std::pair<uint64_t,uint64_t>> CountCells() const;
};

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
bool Interval<Dimension, BitsPerDimension, T>::DataEquals(const Interval<Dimension,BitsPerDimension,T>& rhs) const {
    if constexpr (std::is_same<T, std::monostate>::value) {
        return true;
    } else {
        return data_ == rhs.data_;
    }
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
std::optional<Interval<Dimension,BitsPerDimension,T>> Interval<Dimension, BitsPerDimension, T>::Intersect(const Interval<Dimension,BitsPerDimension,T>& rhs) const {
    uint64_t i_start = std::max(start_, rhs.start_);
    uint64_t i_end = std::min(end_, rhs.end_);
    if (i_start > i_end) {
      return nullptr;
    }
    return std::optional{Interval<Dimension, BitsPerDimension>{i_start, i_end, data_}};
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
uint64_t Interval<Dimension, BitsPerDimension, T>::Area() const {
    assert(start_ <= end_);
    return end_ + 1 - start_;
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
uint64_t Interval<Dimension, BitsPerDimension, T>::StartAlignment() const {
    return start_ != 0 ? __builtin_ctzll(start_) / Dimension : std::numeric_limits<uint64_t>::max();
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
std::vector<Interval<Dimension,BitsPerDimension,T>> Interval<Dimension, BitsPerDimension, T>::ToCells() const {
    uint64_t s = start_;
    std::vector<Interval<Dimension,BitsPerDimension,T>> v = {};
    while(s <= end_){
        auto amax = GetAlignMax<Dimension, BitsPerDimension>(s, end_);
        v.push_back({s, amax, data_});
        s = amax + 1;
    }
    return v;
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
std::vector<Interval<Dimension,BitsPerDimension,T>> Interval<Dimension, BitsPerDimension, T>::ToCells(size_t max_level) const {
    uint64_t s = start_;
    std::vector<Interval<Dimension,BitsPerDimension,T>> v = {};
    while(s <= end_){
        auto amax = std::min(s + (1 << max_level), GetAlignMax<Dimension, BitsPerDimension>(s, end_));
        v.push_back({s, amax, data_});
        s = amax + 1;
    }
    return v;
}

// this returns an sorted map of the cells and size.
// e.g. 3 cells of size 1, 2 cells of size 2, and one cell of size 3.
// where size 1 = 1 on each side, size 2 = 2, size 3  = 4
template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
std::vector<std::pair<uint64_t,uint64_t>> Interval<Dimension, BitsPerDimension, T>::CountCells() const {
    uint64_t s = start_;
    std::vector<std::pair<uint64_t,uint64_t>> v = {};
    while(s <= end_){
        auto amax = GetAlignMax<Dimension, BitsPerDimension>(s, end_);
        auto diff = FastLog2(1 + amax - s) / Dimension;
        size_t i = 0;
        bool found = false;
        for(; i < v.size(); i++){
            if (v[i].first == diff) {
                found = true;
                break;
            }
        }
        if (found) {
            v[i].second += 1;
        } else {
            v.push_back({diff, 1});
        }
        s = amax + 1;
    }
    std::sort(v.begin(), v.end());
    return v;
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
static std::pair<uint64_t,uint64_t> GetParentCell(Interval<Dimension, BitsPerDimension, T> interval){
    auto level = GetUnifyingLevel<Dimension>(interval.start_, interval.end_);
    auto parent = GetParentMortonAligned<Dimension>(interval.start_, level);
    return {parent, level};
}

template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
static Interval<Dimension, BitsPerDimension, T> GetParentCell(const Interval<Dimension, BitsPerDimension, T> interval){
    std::pair<uint64_t,uint64_t> pair = GetParentCell<Dimension>(interval);
    if (std::is_same<T, std::monostate>::value){
        return {pair.first, pair.first + GetMortonCode<Dimension>(pair.second)};
    } else {
        return {pair.first, pair.first + GetMortonCode<Dimension>(pair.second), interval.data};
    }
}

} // ::doux

#endif // ZORDER_INTERVAL_H