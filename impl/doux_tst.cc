#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "impl/zorder/aabb.h"
#include "impl/zorder/encoding.h"


static bool InIntervals(const doux::Region<2, 32>& region, uint64_t code) {
  // 线性扫即可（interval 数量通常不大）；需要更快可改二分。
  for (const auto& itv : region.intervals_) {
    if (code >= static_cast<uint64_t>(itv.start_) &&
        code <= static_cast<uint64_t>(itv.end_)) {
      return true;
    }
  }
  return false;
}

int main() {
  // tpch_q6: shipdate in [0.143192, 0.285994], quantity in [0, 24]
  const uint32_t shipdate_start = static_cast<uint32_t>(0.143192 * 10000);
  const uint32_t shipdate_end = static_cast<uint32_t>(0.285994 * 10000);
  const uint32_t quantity_start = 0;
  const uint32_t quantity_end = 24;

  const uint32_t x_min = std::min(shipdate_start, shipdate_end);
  const uint32_t x_max = std::max(shipdate_start, shipdate_end);
  const uint32_t y_min = std::min(quantity_start, quantity_end);
  const uint32_t y_max = std::max(quantity_start, quantity_end);

  // AABB -> intervals
  doux::AABB<2, 32> aabb = {
      doux::MortonCode<2, 32>::Encode({x_min, y_min}),
      doux::MortonCode<2, 32>::Encode({x_max, y_max}),
  };
  doux::Region<2, 32> region = aabb.ToIntervals();

  std::cout << "x in [" << x_min << "," << x_max << "], y in [" << y_min << ","
            << y_max << "]\n";
  std::cout << "interval count: " << region.intervals_.size() << "\n";

  // 打印 intervals，并检查是否有序/重叠
  bool sorted_non_overlapping = true;
  for (size_t i = 0; i < region.intervals_.size(); ++i) {
    const auto& itv = region.intervals_[i];
    std::cout << i << ": [" << static_cast<uint64_t>(itv.start_) << ", "
              << static_cast<uint64_t>(itv.end_) << "]\n";
    if (i > 0) {
      const auto& prev = region.intervals_[i - 1];
      if (static_cast<uint64_t>(prev.start_) > static_cast<uint64_t>(itv.start_) ||
          static_cast<uint64_t>(prev.end_) >= static_cast<uint64_t>(itv.start_)) {
        sorted_non_overlapping = false;
      }
    }
  }
  std::cout << "intervals sorted & non-overlapping: "
            << (sorted_non_overlapping ? "true" : "false") << "\n";

  // 1) 漏报检查：AABB 内所有点 -> Morton 必须被 intervals 覆盖
  uint64_t miss_cnt = 0;
  uint64_t total_cnt = 0;
  for (uint32_t x = x_min; x <= x_max; ++x) {
    for (uint32_t y = y_min; y <= y_max; ++y) {
      const uint64_t code = static_cast<uint64_t>(doux::MortonCode<2, 32>::Encode({x, y}));
      ++total_cnt;
      if (!InIntervals(region, code)) {
        ++miss_cnt;
        if (miss_cnt <= 10) {
          std::cout << "MISS example: x=" << x << " y=" << y << " code=" << code
                    << "\n";
        }
      }
    }
  }
  std::cout << "total points: " << total_cnt << ", miss_cnt: " << miss_cnt << "\n";

  // 2) 误报检查（可选）：interval 覆盖到的 Morton code decode 后是否落在 AABB
  // 注意：如果你的 ToIntervals() 语义是“允许多覆盖”，这里的 false_cnt > 0 也可能是可接受的。
  uint64_t false_cnt = 0;
  uint64_t checked_cnt = 0;
  for (const auto& itv : region.intervals_) {
    const uint64_t start = static_cast<uint64_t>(itv.start_);
    const uint64_t end = static_cast<uint64_t>(itv.end_);
    for (uint64_t code = start; code <= end; ++code) {
      ++checked_cnt;
      const auto xy = doux::MortonCode<2, 32>::Decode(doux::MortonCode<2, 32>(code));
      const uint32_t x = xy[0];
      const uint32_t y = xy[1];
      if (x < x_min || x > x_max || y < y_min || y > y_max) {
        ++false_cnt;
        if (false_cnt <= 10) {
          std::cout << "FALSE_POS example: code=" << code << " -> x=" << x
                    << " y=" << y << "\n";
        }
      }
      // 防止 interval 太大导致跑爆：这里给个上限（你这组参数通常不会太大）
      if (checked_cnt >= 2000000) {
        std::cout << "Stop false-positive scan early at checked_cnt=" << checked_cnt
                  << " (increase limit if needed)\n";
        code = end;  // break inner
      }
    }
    if (checked_cnt >= 2000000) break;
  }
  std::cout << "checked interval codes: " << checked_cnt
            << ", false_cnt (optional): " << false_cnt << "\n";

  return (miss_cnt == 0) ? 0 : 2;
}