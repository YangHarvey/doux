#ifndef ZORDER_REGION_H
#define ZORDER_REGION_H

#include <cstdint>
#include <cassert>

#include <algorithm>
#include <vector>
#include <tuple>
#include <variant>
#include <type_traits>

#include "impl/zorder/encoding.h"
#include "impl/zorder/interval.h"
#include <immintrin.h>

namespace doux {
template <uint32_t Dimension, uint32_t BitsPerDimension, typename T = std::monostate>
struct Region {
    using interval_type = Interval<Dimension, BitsPerDimension, T>;
    std::vector<interval_type> intervals_;

    template<typename M = std::monostate>
    friend bool operator==(const Region& lhs, const Region<Dimension, BitsPerDimension, M>& rhs) {
        return lhs.intervals_ == rhs.intervals_;
    }

    friend Region operator|(const Region& lhs, const Region& rhs) {
        Region r = lhs;
        r |= rhs;
        return r;
    }

    template<typename M = std::monostate>
    friend Region operator&(const Region& lhs, const Region<Dimension, BitsPerDimension, M>& rhs) {
        Region r = lhs;
        r &= rhs;
        return r;
    }

    friend void operator|=(Region& lhs, const Region& rhs) {
        {
            assert(std::is_sorted(lhs.intervals_.begin(), lhs.intervals_.end()) && std::is_sorted(rhs.intervals_.begin(), rhs.intervals_.end()));
            size_t mid = lhs.intervals_.size();
            lhs.intervals_.insert(lhs.intervals_.end(), rhs.intervals_.begin(), rhs.intervals_.end());
            std::inplace_merge(lhs.intervals_.begin(), lhs.intervals_.begin() + mid, lhs.intervals_.end());
        }
        std::vector<bool> to_delete(lhs.intervals_.size());
        for (size_t i = 0; i < lhs.intervals_.size() - 1; i++) {
            auto end = lhs.intervals_[i].end;
            size_t j;
            for (j = i + 1;
                j < lhs.intervals_.size() &&
                lhs.intervals_[j].start-1 <= end && lhs.intervals_[j].data_equals(lhs.intervals_[i])
            ; j++) {
                end = std::max(end, lhs.intervals_[j].end);
            }
            lhs.intervals_[i].end = end;
            for (size_t k = i+1; k < j; k++) {
                to_delete[k] = true;
            }
            i = j-1; // to skip over the merged intervals_
        }
        auto it = to_delete.begin();
        lhs.intervals_.erase(
            std::remove_if(lhs.intervals_.begin(), lhs.intervals_.end(),
                [&it](const Interval<Dimension, BitsPerDimension, T>& d) -> bool { return *it++; }
            )
        , lhs.intervals_.end());
    }

    // this assumes regions contain a sorted list of morton intervals_
    template<typename M = std::monostate>
    friend void operator&=(Region& lhs, const Region<Dimension, BitsPerDimension, M>& rhs) {
        assert(std::is_sorted(lhs.intervals_.begin(), lhs.intervals_.end()) && std::is_sorted(rhs.intervals_.begin(), rhs.intervals_.end()));
        auto lhs_it = lhs.intervals_.begin();
        auto rhs_it = rhs.intervals_.begin();
        std::vector<Interval<Dimension, BitsPerDimension, T>> out = {};
        while(lhs_it != lhs.intervals_.end() && rhs_it != rhs.intervals_.end()){
            if (lhs_it->end < rhs_it->start) {
                ++lhs_it;
                continue;
            } else if (rhs_it->end < lhs_it->start) {
                ++rhs_it;
                continue;
            }
            auto s = std::max(lhs_it->start, rhs_it->start);
            auto e = std::min(lhs_it->end, rhs_it->end);
            out.push_back(Interval<Dimension, BitsPerDimension, T>{s,e, lhs_it->data});
            if (lhs_it->end < rhs_it->end){
                ++lhs_it;
            } else if(rhs_it->end < lhs_it->end) {
                ++rhs_it;
            } else {
                ++rhs_it;
                ++lhs_it;
            }
        }
        lhs.intervals_ = out;
    }

    template<typename M = std::monostate>
    friend void operator-=(Region& lhs, const Region<Dimension, BitsPerDimension, M>& rhs) {
        assert(std::is_sorted(lhs.intervals_.begin(), lhs.intervals_.end()) && std::is_sorted(rhs.intervals_.begin(), rhs.intervals_.end()));
        auto lhs_it = lhs.intervals_.begin();
        auto rhs_it = rhs.intervals_.begin();
        std::vector<Interval<Dimension, BitsPerDimension, T>> out {};
        MortonCode<2, 32> s{0};
        if (lhs_it != lhs.intervals_.end()) {
             s = lhs_it->start;
        }
        while(lhs_it != lhs.intervals_.end() && rhs_it != rhs.intervals_.end()){
            if (lhs_it->end < rhs_it->start ){ //if the lhs is entirely behind the rhs push the lhs
                // lhs: |------|
                // rhs:          |--|
                out.push_back(Interval<Dimension, BitsPerDimension, T>{s,lhs_it->end, lhs_it->data});
                ++lhs_it;
                if (lhs_it != lhs.intervals_.end()) {
                    s = lhs_it->start;
                }
                continue;
            }
            if (s > rhs_it->end){ // if the rhs is entirely behind the lhs, push the rhs
                // lhs:      |---|
                // rhs:|--|
                ++rhs_it;
                continue;
            }
            // we know now that the rhs collides with the lhs
            if (s >= rhs_it->start){
                //      A       |  |     B
                // lhs:  |---|  |or|     |---|
                // rhs:|--|     |  | |-------|
                if (lhs_it->end <= rhs_it->end){ // case B - move onto the next interval
                    ++lhs_it;
                    if (lhs_it != lhs.intervals_.end()) {
                        s = lhs_it->start;
                    }
                } else { // case A - split the interval and check the next rhs segment
                    s = rhs_it->end + 1;
                    ++rhs_it;
                }
                continue;
            }
            // s must be < rhs
            //      A     |  |     B
            // lhs:|---|  |or| |---|
            // rhs: |-|   |  |   |----|
            out.push_back({s,rhs_it->start-1,lhs_it->data});
            if (rhs_it->end < lhs_it->end) { // case A
                s = rhs_it->end +1;
                ++rhs_it;
            } else { // case B
                ++lhs_it;
                if (lhs_it != lhs.intervals_.end()) {
                    s = lhs_it->start;
                }
            }
        }
        if (lhs_it != lhs.intervals_.end()) {
            //push the last one, then copy the rest
            out.push_back({s,lhs_it->end, lhs_it->data});
            out.insert(out.end(), ++lhs_it, lhs.intervals_.end());
        }
        lhs.intervals_ = out;
    }

    template<typename M = std::monostate>
    friend Region operator-(const Region& lhs, const Region<Dimension, BitsPerDimension, M>& rhs) {
        Region x = lhs;
        x -= rhs;
        return x;
    }

    template<typename M>
    bool Intersects(const Region<Dimension, BitsPerDimension, M>& rhs) const;
    bool Empty() const;
    uint64_t Area() const;
    bool Contains(const MortonCode<Dimension, BitsPerDimension> c) const {
        for (auto& i: intervals_) {
            if (c.code < i.start) {
                return false;
            } else if (c.code <= i.end) {
                return true;
            }
        }
        return false;
    };
    std::vector<Interval<Dimension, BitsPerDimension>> ToCells() const;
    std::vector<Interval<Dimension, BitsPerDimension>> ToCells(size_t max_level) const;
    std::vector<std::pair<uint64_t,uint64_t>> CountCells() const;
};

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    template<typename M>
    bool Region<Dimension, BitsPerDimension, T>::Intersects(const Region<Dimension, BitsPerDimension, M>& rhs) const {
        assert(std::is_sorted(intervals_.begin(), intervals_.end()) && std::is_sorted(rhs.intervals_.begin(), rhs.intervals_.end()));
        auto lhs_it = intervals_.begin();
        auto rhs_it = rhs.intervals_.begin();
        while(lhs_it != intervals_.end() && rhs_it != rhs.intervals_.end()){
            if (lhs_it->end < rhs_it->start) {
                ++lhs_it;
                continue;
            } else if (rhs_it->end < lhs_it->start) {
                ++rhs_it;
                continue;
            }
            return true;
        }
        return false;
    }

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    bool Region<Dimension, BitsPerDimension, T>::Empty() const {
        return intervals_.empty();
    }

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    uint64_t Region<Dimension, BitsPerDimension, T>::Area() const {
        uint64_t a = 0;
        for(auto &cell : intervals_){
            a += cell.Area();
        }
        return a;
    }

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    std::vector<Interval<Dimension, BitsPerDimension>> Region<Dimension, BitsPerDimension, T>::ToCells() const {
        std::vector<Interval<Dimension, BitsPerDimension>> v = {};
        for (auto &i : intervals_){
            auto c = i.ToCells();
            v.insert(v.end(), c.begin(), c.end());
        }
        return v;
    }

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    std::vector<Interval<Dimension, BitsPerDimension>> Region<Dimension, BitsPerDimension, T>::ToCells(size_t max_level) const {
        std::vector<Interval<Dimension, BitsPerDimension>> v = {};
        for (auto &i : intervals_){
            auto c = i.ToCells(max_level);
            v.insert(v.end(), c.begin(), c.end());
        }
        return v;
    }

    template<uint32_t Dimension, uint32_t BitsPerDimension, typename T>
    std::vector<std::pair<uint64_t,uint64_t>> Region<Dimension, BitsPerDimension, T>::CountCells() const {
        std::vector<std::pair<uint64_t,uint64_t>> counts = {};
        if (intervals_.empty()) { return counts;}
        counts = intervals_[0].CountCells();
        for(size_t i = 1; i < intervals_.size(); i++){
            auto v = intervals_[i].CountCells();
            for(auto it = v.begin(); it != v.end(); ++it){
                size_t j = 0;
                for(; j < counts.size(); j++){
                    if(counts[j].first < it->first && j < counts.size() -1 && counts[j+1].first > it->first){
                        //insert if between two existing values
                        counts.emplace(counts.begin()+j,*it);
                        break;
                    }
                    if(counts[j].first == it->first){
                        counts[j].second += it->second;
                        break;
                    }
                }
                if (j == counts.size()){
                    counts.push_back(*it);
                }
            }
        }
        return counts;
    }

template<typename T>
static Region<2,32,T> CellToRegion(uint64_t code, uint64_t level, T data) {
    if constexpr (std::is_same<T, std::monostate>::value) {
        return {{{code, code + (1 << (level * 2)) - 1}}};
    } else {
        return {{{code, code + (1 << (level * 2)) - 1, data}}};
    }
}

} //::doux

#endif // ZORDER_REGION_H