#ifndef ZORDER_AABB_H
#define ZORDER_AABB_H

#include <cstdint>
#include <cassert>

#include <algorithm>
#include <vector>
#include <tuple>
#include <variant>
#include <type_traits>

#include "impl/zorder/encoding.h"
#include "impl/zorder/region.h"
#include <immintrin.h>

namespace doux {
template<uint32_t Dimension, uint32_t BitsPerDimension>
struct AABB {
    // AABBs can have min = max represent a single interval.
    // That is min and max are inclusive values
    MortonCode<Dimension, BitsPerDimension> min_, max_;

    AABB(MortonCode<Dimension, BitsPerDimension> min, MortonCode<Dimension, BitsPerDimension> max): min_(min), max_(max) {};

    class IteratorIntervals {
        public:
            typedef std::input_iterator_tag iterator_category;
            typedef Interval<Dimension, BitsPerDimension> value_type;
            typedef const value_type *pointer;
            typedef const value_type &reference;

        private:

            size_t iterator_index_;
            value_type value_;
            value_type curr_;
            const AABB &parent_aabb_;

            std::vector<AABB> inputs_;

            bool IsEnd() const {
                return is_finished_;
            }

            void Progress() {
                while (!inputs_.empty()) {
                    AABB aabb = inputs_.back();
                    inputs_.pop_back();
                    if (aabb.IsMortonAligned()) {
                        // This is checking that curr is uninitialised
                        // in the case that parent_aabb = {0,0} this still works as
                        // curr is always assigned to the value at the end.
                        if (curr_ == Interval<Dimension, BitsPerDimension>{0,0}){
                            curr_ = aabb.ToCell();
                            continue;
                        } else if (curr_.end_ + 1 == aabb.min_) {
                            curr_.end_ = aabb.max_;
                            continue;
                        } else {
                            value_ = curr_;
                            curr_ = aabb.ToCell();
                            iterator_index_++;
                            return;
                        }
                    }
                    auto [litmax, bigmin] = aabb.MortonGetNextAddress();
                    AABB first = {aabb.min_, litmax};
                    AABB second = {bigmin, aabb.max_};
                    assert(first.max_ >= first.min_);
                    assert(second.max_ >= second.min_);
                    inputs_.push_back(second);
                    inputs_.push_back(first);
                }
                value_ = curr_;
                is_finished_ = true;
                return;
            }


        public:
            bool is_finished_;

            IteratorIntervals(const AABB &parent): value_({0, 0}), curr_({0, 0}), parent_aabb_(parent) {
                iterator_index_ = 0;
                inputs_ = {parent};
                is_finished_ = false;
                Progress();
            }

            void SetEnd() {
                is_finished_ = true;
            }

            IteratorIntervals &operator++() {
                Progress();
                return *this;
            }

            bool operator==(const IteratorIntervals &i) const {
                return parent_aabb_ == i.parent_aabb_ && is_finished_ == i.is_finished_ && (is_finished_ == true || iterator_index_ == i.iterator_index_);
            }

            bool operator!=(const IteratorIntervals &i) const {
                return !(*this == i);
            }

            reference operator*() const {
                return value_;
            }

            pointer operator->() const {
                return &value_;
            }
    };

    IteratorIntervals Begin() const {
        return IteratorIntervals(*this);
    }

    IteratorIntervals End() const {
        auto i = IteratorIntervals(*this);
        i.SetEnd();
        return i;
    }

    friend bool operator==(const AABB& lhs, const AABB& rhs){
        return lhs.min_ == rhs.min_ && lhs.max_ == rhs.max_;
    }

    bool IsMortonAligned() const;

    Interval<Dimension, BitsPerDimension> ToCell() const;

    // This generates a list of all morton aligned intervals (morton cells)
    // that are within the AABB
    // it generates them in a sorted order, from lowest interval to highest.
    Region<Dimension, BitsPerDimension> ToCells() const;

    // This generates a list of all contiguous morton intervals (these are not necessarily aligned)
    // that are within the AABB
    // it generates them in a sorted order, from lowest interval to highest.
    Region<Dimension, BitsPerDimension> ToIntervals() const;

    uint64_t GetNextMortonOutside(uint64_t m) const;

    uint64_t GetNextMortonInside(uint64_t m) const;

    std::pair<MortonCode<Dimension, BitsPerDimension>, MortonCode<Dimension, BitsPerDimension>> MortonGetNextAddress();
};

template<uint32_t Dimension, uint32_t BitsPerDimension>
bool AABB<Dimension, BitsPerDimension>::IsMortonAligned() const {
    assert(max_ >= min_);
    uint64_t align_max = min_ != 0 ? __builtin_ctzll(min_) : std::numeric_limits<uint64_t>::max();
    uint64_t diff = max_ - min_ + 1;
    uint64_t align = __builtin_ctzll(diff);
    return
        align / Dimension <= align_max / Dimension &&
        __builtin_popcountll(diff) == 1 &&
        align % Dimension == 0;
}

template<uint32_t Dimension, uint32_t BitsPerDimension>
Interval<Dimension, BitsPerDimension> AABB<Dimension, BitsPerDimension>::ToCell() const {
    assert(this->IsMortonAligned());
    return Interval<Dimension, BitsPerDimension>{min_, max_};
}

// This generates a list of all morton aligned intervals (morton cells)
// that are within the AABB
// it generates them in a sorted order, from lowest interval to highest.
template<uint32_t Dimension, uint32_t BitsPerDimension>
Region<Dimension, BitsPerDimension> AABB<Dimension, BitsPerDimension>::ToCells() const {
    assert(max_ >= min_);
    std::vector<AABB> inputs {*this};
    std::vector<Interval<Dimension, BitsPerDimension>> outputs;
    while (!inputs.empty()) {
        AABB aabb = inputs.back();
        inputs.pop_back();
        if (aabb.IsMortonAligned()) {
            outputs.push_back(aabb.ToCell());
            continue;
        }
        auto [litmax, bigmin] = aabb.MortonGetNextAddress();
        AABB first = {aabb.min_, litmax};
        AABB second = {bigmin, aabb.max_};
        assert(first.max_ >= first.min_);
        assert(second.max_ >= second.min_);
        inputs.push_back(second);
        inputs.push_back(first);
    }
    return {outputs};
}

// This generates a list of all contiguous morton intervals (these are not necessarily aligned)
// that are within the AABB
// it generates them in a sorted order, from lowest interval to highest.
template<uint32_t Dimension, uint32_t BitsPerDimension>
Region<Dimension, BitsPerDimension> AABB<Dimension, BitsPerDimension>::ToIntervals() const {
    assert(max_ >= min_);
    std::vector<AABB> inputs {*this};
    std::vector<Interval<Dimension, BitsPerDimension>> outputs;
    while (!inputs.empty()) {
        AABB aabb = inputs.back();
        inputs.pop_back();
        if (aabb.IsMortonAligned()) {
            //if the cell generated connects to the previous cell, merge.
            if (!outputs.empty() && outputs.back().end_ + 1 == aabb.min_) {
                outputs.back().end_ = aabb.max_;
            } else {
                outputs.push_back(aabb.ToCell());
            }
            continue;
        }
        auto [litmax, bigmin] = aabb.MortonGetNextAddress();
        AABB first = {aabb.min_, litmax};
        AABB second = {bigmin, aabb.max_};
        assert(first.max_ >= first.min_);
        assert(second.max_ >= second.min_);
        inputs.push_back(second);
        inputs.push_back(first);
    }
    return {outputs};
}

template<uint32_t Dimension, uint32_t BitsPerDimension>
uint64_t AABB<Dimension, BitsPerDimension>::GetNextMortonOutside(uint64_t m) const {
    uint64_t min_x, min_y, max_x, max_y, x, y;
    auto pack = MortonCode<Dimension, BitsPerDimension>::decode({min_});
    min_x = pack[0];
    min_y = pack[1];
    pack = MortonCode<Dimension, BitsPerDimension>::decode({max_});
    max_x = pack[0];
    max_y = pack[1];
    pack = MortonCode<Dimension, BitsPerDimension>::decode({m});
    x = pack[0];
    y = pack[1];
    assert(x == min_x || y == min_y);
    max_x += 1;
    max_y += 1;
    min_x = std::min(min_x, (uint64_t)(1LLU << 63));
    min_y = std::min(min_y, (uint64_t)(1LLU << 63));
    uint64_t l = std::min({__builtin_ctzll(min_x), __builtin_ctzll(min_y), __builtin_ctzll(max_x), __builtin_ctzll(max_x)}) * Dimension;
    m = (m >> l) << l;
    m = m + (1LLU << l);
    return m;
}

template<uint32_t Dimension, uint32_t BitsPerDimension>
uint64_t AABB<Dimension, BitsPerDimension>::GetNextMortonInside(uint64_t m) const {
    return 0;
}

template<uint32_t Dimension, uint32_t BitsPerDimension>
std::pair<MortonCode<Dimension, BitsPerDimension>, MortonCode<Dimension, BitsPerDimension>> AABB<Dimension, BitsPerDimension>::MortonGetNextAddress() {
    static_assert(Dimension == 2, "this needs more care to convert to 3D");
    uint64_t litmax = max_;
    uint64_t bigmin = min_;
    uint64_t index = 65 - __builtin_clzll(min_ ^ max_);
    uint64_t mask = ~((1LLU << (index / 2)) - 1);
    uint64_t inc = 1LLU << ((index / 2) - 1);
    index %= 2;

    uint32_t part = (compact_bits_2(min_ >> index) & mask) + inc;
    bigmin &= ~(__morton_2_x_mask << index);
    bigmin |= expand_bits_2((uint64_t)part) << index;

    part -= 1;
    litmax &= ~(__morton_2_x_mask << index);
    litmax |= expand_bits_2((uint64_t)part) << index;

    return std::pair<MortonCode<Dimension, BitsPerDimension>, MortonCode<Dimension, BitsPerDimension>>({litmax}, {bigmin});
}

} //::doux

#endif