#ifndef ZORDER_ENCODING_H
#define ZORDER_ENCODING_H

#include <cstdint>
#include <cassert>
#include <immintrin.h>
#include <array>
#include <tuple>

#include "leveldb/slice.h"
#include "leveldb/comparator.h"
#include "impl/zorder/util.h"

namespace doux {
template<uint32_t Dimension, uint32_t BitsPerDimension>
struct MortonCode {
    static_assert((Dimension == 2 && BitsPerDimension == 32) || (Dimension == 3 && BitsPerDimension == 21),
        "only 2D and 32 bits, or 3D and 21 bits are currently supported.");
};

template<>
struct MortonCode<2, 32> {
    uint64_t data_;
    static constexpr uint32_t dimension_ = 2;
    static constexpr uint32_t max_level_ = 32;

    MortonCode<2, 32>(uint64_t data) : data_(data) {};

    operator uint64_t() const {
        return data_;
    }

    static MortonCode<2, 32> Encode(std::array<uint32_t, 2> p) {
        return {
            (expand_bits_2<uint64_t>(std::get<0>(p)) << 0) |
            (expand_bits_2<uint64_t>(std::get<1>(p)) << 1)
        };
    }

    static std::array<uint32_t, 2> Decode(const struct MortonCode<2, 32> code) {
        return {
            static_cast<uint32_t>(compact_bits_2<uint64_t>(code.data_ >> 0)),
            static_cast<uint32_t>(compact_bits_2<uint64_t>(code.data_ >> 1)),
        };
    }

    friend void operator-=(MortonCode<2, 32>& lhs, const MortonCode<2, 32>& rhs) {
        uint64_t x = (lhs.data_ & __morton_2_x_mask) - (rhs.data_ & __morton_2_x_mask);
        uint64_t y = (lhs.data_ & __morton_2_y_mask) - (rhs.data_ & __morton_2_y_mask);
        lhs.data_ = (x & __morton_2_x_mask) | (y & __morton_2_y_mask);
    }

    friend void operator+=(MortonCode<2, 32>& lhs, const MortonCode<2, 32>& rhs) {
        uint64_t x = (lhs.data_ | ~__morton_2_x_mask) + (rhs.data_ & __morton_2_x_mask);
        uint64_t y = (lhs.data_ | ~__morton_2_y_mask) + (rhs.data_ & __morton_2_y_mask);
        lhs.data_ = (x & __morton_2_x_mask) | (y & __morton_2_y_mask);
    }
};

template<>
struct MortonCode<3, 21> {
    uint64_t data_;
    static constexpr uint32_t dimension_ = 3;
    static constexpr uint32_t max_level_ = 21;

    static MortonCode<3, 21> Encode(std::array<uint32_t, 3> p) {
        return {
            (expand_bits_2<uint64_t>(std::get<0>(p)) << 0) |
            (expand_bits_2<uint64_t>(std::get<1>(p)) << 1) |
            (expand_bits_2<uint64_t>(std::get<2>(p)) << 2)
        };
    }

    static std::array<uint32_t, 3> decode(const struct MortonCode<3, 21> code) {
        return {
            static_cast<uint32_t>(compact_bits_2<uint64_t>(code.data_ >> 0)),
            static_cast<uint32_t>(compact_bits_2<uint64_t>(code.data_ >> 1)),
            static_cast<uint32_t>(compact_bits_2<uint64_t>(code.data_ >> 2)),
        };
    }

    friend void operator+=(MortonCode<3, 21>& lhs, const MortonCode<3, 21>& rhs) {
        uint64_t x = (lhs.data_ | ~__morton_3_x_mask) + (rhs.data_ & __morton_3_x_mask);
        uint64_t y = (lhs.data_ | ~__morton_3_y_mask) + (rhs.data_ & __morton_3_y_mask);
        uint64_t z = (lhs.data_ | ~__morton_3_z_mask) + (rhs.data_ & __morton_3_z_mask);
        lhs.data_ = (x & __morton_3_x_mask) | (y & __morton_3_y_mask) | (z & __morton_3_z_mask);
    }

    friend void operator-=(MortonCode<3, 21>& lhs, const MortonCode<3, 21>& rhs) {
        uint64_t x = (lhs.data_ & __morton_3_x_mask) - (rhs.data_ & __morton_3_x_mask);
        uint64_t y = (lhs.data_ & __morton_3_y_mask) - (rhs.data_ & __morton_3_y_mask);
        uint64_t z = (lhs.data_ & __morton_3_z_mask) - (rhs.data_ & __morton_3_z_mask);
        lhs.data_ = (x & __morton_3_x_mask) | (y & __morton_3_y_mask) | (z & __morton_3_z_mask);
    }
};

}

#endif  // ZORDER_ENCODING_H