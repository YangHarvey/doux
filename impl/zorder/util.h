#ifndef ZORDER_UTIL_H
#define ZORDER_UTIL_H

#include <cstdint>
#include <cassert>

#include <immintrin.h>

namespace doux {

static uint64_t FastLog2(const uint64_t x) {
    assert(x != 0);
    return sizeof(x) * 8 - 1 - __builtin_clzll(x);
}

// Returns the maximum alignment level possible for a given number
template<uint32_t Dimension, uint32_t BitsPerDimension>
static uint64_t GetMaxAlignLevel(const uint64_t code) {
    return code == 0 ? BitsPerDimension : __builtin_ctzll(code) / Dimension;
}

// Returns the level size required for two points to be in the same cell
template<uint32_t Dimension>
static uint64_t GetUnifyingLevel(const uint64_t min, const uint64_t max) {
    assert(max >= min);
    // the gets the maximum allowed for this range, max + 1
    return max == min ? 0 : (FastLog2(max ^ min) / Dimension) + 1;
}

// Returns the morton code (i.e., zorder) for a given level, starting from 0
template<uint32_t Dimension>
static uint64_t GetMortonCode(const uint64_t level) {
    return (1LLU << level * Dimension) - 1;
}

// Given a mortan value it will return the aligned value of size level that contains it
template<uint32_t Dimension>
static uint64_t GetParentMortonAligned(const uint64_t code, uint32_t level) {
    return (code >> (Dimension * level)) << (Dimension * level);
}

// Given a range this will return the next morton aligned value
template<uint32_t Dimension, uint32_t BitsPerDimension>
static uint64_t GetAlignMax(const uint64_t min, const uint64_t max) {
    assert(max >= min);
    if (max == min) return min;
    // This gets the maximum allowed for this value
    uint64_t align_max = (min != 0) ? 
          min + GetMortonCode<Dimension>(GetMaxAlignLevel<Dimension, BitsPerDimension>(min)) 
        : std::numeric_limits<uint64_t>::max();
    uint64_t max_align = min + GetMortonCode<Dimension>(FastLog2(max + 1 - min) / Dimension);
    return std::min(align_max, max_align);
}

static const uint64_t __morton_2_x_mask = 0x5555555555555555;
static const uint64_t __morton_2_y_mask = 0xaaaaaaaaaaaaaaaa;

static const uint64_t __morton_3_x_mask = 0x1249249249249249;
static const uint64_t __morton_3_y_mask = 0x2492492492492492;
static const uint64_t __morton_3_z_mask = 0x4924924924924924;

#ifdef __BMI2__
template<typename Integer>
static inline constexpr Integer pdep(Integer v, Integer mask) {
    static_assert(sizeof(Integer) == 4 || sizeof(Integer) == 8);
    if constexpr (sizeof(Integer) == 4)
        return _pdep_u32(v, mask);
    else if constexpr (sizeof(Integer) == 8)
        return _pdep_u64(v, mask);
}

template<typename Integer>
static inline constexpr Integer pext(Integer v, Integer mask) {
    static_assert(sizeof(Integer) == 4 || sizeof(Integer) == 8);
    if constexpr (sizeof(Integer) == 4)
        return _pext_u32(v, mask);
    else if constexpr (sizeof(Integer) == 8)
        return _pext_u64(v, mask);
}

template<typename Integer>
static inline constexpr Integer expand_bits_2(Integer v) {
    return pdep(v, __morton_2_x_mask);
}

template<typename Integer>
static inline constexpr Integer compact_bits_2(Integer v) {
    return pext(v, __morton_2_x_mask);
}

template<typename Integer>
static inline constexpr Integer expand_bits_3(Integer v) {
    return pdep(v, __morton_3_x_mask);
}

template<typename Integer>
static inline constexpr Integer compact_bits_3(Integer v) {
    return pext(v, __morton_3_x_mask);
}
#else
template<typename Integer>
static inline Integer expand_bits_2(Integer x) {
    static_assert(sizeof(Integer) == 8, "non 64bit non BMI2 expand/compact_bits_2 are not yet implemented");
    x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
    x = (x ^ (x << 8))  & 0x00ff00ff00ff00ff;
    x = (x ^ (x << 4))  & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x << 2))  & 0x3333333333333333;
    x = (x ^ (x << 1))  & 0x5555555555555555;
    return x;
}

template<typename Integer>
static inline Integer compact_bits_2(Integer x) {
    x &= 0x5555555555555555;
    x = (x ^ (x >>  1))  & 0x3333333333333333;
    x = (x ^ (x >>  2))  & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x >>  4))  & 0x00ff00ff00ff00ff;
    x = (x ^ (x >>  8))  & 0x0000ffff0000ffff;
    x = (x ^ (x >>  16)) & 0x00000000ffffffff;
    return x;
}
#endif


} // namespace zorder


#endif //ZORDER_UTIL_H