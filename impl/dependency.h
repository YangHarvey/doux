#ifndef DOUX_DEPENDENCY_H
#define DOUX_DEPENDENCY_H

#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <string>

// 前向声明，避免依赖整个 leveldb
namespace leveldb {
  class Status;
}

namespace doux {

// Block range in a VSST file
struct BlockRange {
    uint32_t start_block;  // 起始 block id
    uint32_t end_block;    // 结束 block id (inclusive)
    
    BlockRange() : start_block(0), end_block(0) {}
    BlockRange(uint32_t start, uint32_t end) 
        : start_block(start), end_block(end) {}
    
    bool Contains(uint32_t block_id) const {
        return block_id >= start_block && block_id <= end_block;
    }
    
    uint32_t Size() const {
        return end_block - start_block + 1;
    }
};

// Mapping entry: where blocks have been migrated to
struct MappingEntry {
    uint64_t new_file_number;  // 新的 VSST file number
    BlockRange new_block_range; // 在新文件中的 block 范围
    uint32_t ref_count;         // 引用计数
    
    MappingEntry() : new_file_number(0), ref_count(0) {}
    MappingEntry(uint64_t file_num, const BlockRange& range, uint32_t count = 1)
        : new_file_number(file_num), new_block_range(range), ref_count(count) {}
};

// Block location identifier
struct BlockLocation {
    uint64_t file_number;
    uint32_t block_id;
    
    BlockLocation() : file_number(0), block_id(0) {}
    BlockLocation(uint64_t file, uint32_t block) 
        : file_number(file), block_id(block) {}
    
    bool operator==(const BlockLocation& other) const {
        return file_number == other.file_number && block_id == other.block_id;
    }
    
    bool operator<(const BlockLocation& other) const {
        if (file_number != other.file_number) {
            return file_number < other.file_number;
        }
        return block_id < other.block_id;
    }
};

// Hash function for BlockLocation
struct BlockLocationHash {
    size_t operator()(const BlockLocation& loc) const {
        return std::hash<uint64_t>()(loc.file_number) ^ 
               (std::hash<uint32_t>()(loc.block_id) << 1);
    }
};

// Dependency Graph for tracking VSST block migrations
class DependencyGraph {
public:
    DependencyGraph() {}
    
    // 添加一个映射：origin blocks -> new location
    // origin_file: 原始 VSST 文件号
    // origin_range: 原始 block 范围
    // new_file: 新 VSST 文件号
    // new_range: 新 block 范围
    void AddMapping(uint64_t origin_file, const BlockRange& origin_range,
                   uint64_t new_file, const BlockRange& new_range) {
        assert(origin_range.Size() == new_range.Size());
        
        for (uint32_t i = 0; i < origin_range.Size(); ++i) {
            BlockLocation origin_loc(origin_file, origin_range.start_block + i);
            BlockRange single_block(new_range.start_block + i, new_range.start_block + i);
            
            mappings_[origin_loc] = MappingEntry(new_file, single_block, 1);
        }
    }
    
    // 批量添加映射（用于整个 block range）
    // 这个方法会为 range 内的每个 block 创建独立映射，支持级联查找
    void AddRangeMapping(uint64_t origin_file, const BlockRange& origin_range,
                        uint64_t new_file, const BlockRange& new_range) {
        assert(origin_range.Size() == new_range.Size());
        
        // 为范围内的每个 block 创建映射，这样可以支持级联查找
        for (uint32_t i = 0; i < origin_range.Size(); ++i) {
            BlockLocation origin_loc(origin_file, origin_range.start_block + i);
            BlockRange single_block(new_range.start_block + i, new_range.start_block + i);
            mappings_[origin_loc] = MappingEntry(new_file, single_block, 1);
        }
    }
    
    // 查找一个 block 的最新位置（支持链式级联查找）
    // 返回 true 如果找到重定向，false 如果没有（说明在原始位置）
    // redirect_count: 可选参数，记录经过了多少层重定向
    bool LookupBlock(uint64_t file_number, uint32_t block_id,
                    uint64_t* new_file, uint32_t* new_block,
                    int* redirect_count = nullptr) const {
        BlockLocation loc(file_number, block_id);
        uint64_t original_file = file_number;
        uint32_t original_block = block_id;
        
        int redirects = 0;
        // 最多跟踪 10 层重定向（避免无限循环）
        const int max_redirect = 10;
        
        while (redirects < max_redirect) {
            auto it = mappings_.find(loc);
            if (it == mappings_.end()) {
                // 没有找到映射，说明已经到达最终位置
                *new_file = loc.file_number;
                *new_block = loc.block_id;
                if (redirect_count) *redirect_count = redirects;
                return loc.file_number != original_file || loc.block_id != original_block;
            }
            
            const MappingEntry& entry = it->second;
            
            // 计算在新 range 中的偏移
            // 注意：it->first 是映射的 key（origin location）
            uint32_t offset = loc.block_id - it->first.block_id;
            
            // 检查 offset 是否在有效范围内
            if (offset >= entry.new_block_range.Size()) {
                // 这不应该发生，说明数据不一致
                *new_file = loc.file_number;
                *new_block = loc.block_id;
                if (redirect_count) *redirect_count = redirects;
                return loc.file_number != original_file || loc.block_id != original_block;
            }
            
            // 更新到新位置，继续查找
            loc.file_number = entry.new_file_number;
            loc.block_id = entry.new_block_range.start_block + offset;
            redirects++;
        }
        
        // 达到最大重定向次数，可能有循环
        *new_file = loc.file_number;
        *new_block = loc.block_id;
        if (redirect_count) *redirect_count = redirects;
        return true;
    }
    
    // 减少引用计数（当 KO-tree 中的 vptr 被更新时调用）
    void DecrementRef(uint64_t file_number, uint32_t block_id) {
        BlockLocation loc(file_number, block_id);
        auto it = mappings_.find(loc);
        if (it != mappings_.end()) {
            assert(it->second.ref_count > 0);
            it->second.ref_count--;
            
            // 引用计数为 0，可以删除这个映射
            if (it->second.ref_count == 0) {
                mappings_.erase(it);
            }
        }
    }
    
    // 批量减少引用计数（用于 range）
    void DecrementRangeRef(uint64_t file_number, const BlockRange& range) {
        for (uint32_t block_id = range.start_block; block_id <= range.end_block; ++block_id) {
            DecrementRef(file_number, block_id);
        }
    }
    
    // 垃圾回收：移除所有引用计数为 0 的映射
    size_t GarbageCollect() {
        size_t removed = 0;
        for (auto it = mappings_.begin(); it != mappings_.end();) {
            if (it->second.ref_count == 0) {
                it = mappings_.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }
    
    // 持久化到文件
    bool SaveToFile(const std::string& filename) const {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "Cannot open file for writing: " << filename << std::endl;
            return false;
        }
        
        // 写入版本号
        uint32_t version = 1;
        ofs.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // 写入映射数量
        uint32_t count = static_cast<uint32_t>(mappings_.size());
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        // 写入每个映射
        for (const auto& pair : mappings_) {
            const BlockLocation& loc = pair.first;
            const MappingEntry& entry = pair.second;
            
            ofs.write(reinterpret_cast<const char*>(&loc.file_number), sizeof(loc.file_number));
            ofs.write(reinterpret_cast<const char*>(&loc.block_id), sizeof(loc.block_id));
            ofs.write(reinterpret_cast<const char*>(&entry.new_file_number), sizeof(entry.new_file_number));
            ofs.write(reinterpret_cast<const char*>(&entry.new_block_range.start_block), sizeof(entry.new_block_range.start_block));
            ofs.write(reinterpret_cast<const char*>(&entry.new_block_range.end_block), sizeof(entry.new_block_range.end_block));
            ofs.write(reinterpret_cast<const char*>(&entry.ref_count), sizeof(entry.ref_count));
        }
        
        ofs.close();
        return true;
    }
    
    // 从文件恢复
    // 如果文件不存在，创建一个新的空的 DependencyGraph 并保存到文件
    bool LoadFromFile(const std::string& filename) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs.is_open()) {
            // 文件不存在，创建一个新的空的 DependencyGraph
            mappings_.clear();
            // 保存空文件到磁盘
            if (!SaveToFile(filename)) {
                std::cerr << "Failed to create empty dependency graph file: " << filename << std::endl;
                return false;
            }
            return true;
        }
        
        mappings_.clear();
        
        // 读取版本号
        uint32_t version;
        ifs.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!ifs.good() || version != 1) {
            ifs.close();
            std::cerr << "Unknown dependency graph version or file corrupted: " << filename << std::endl;
            // 文件损坏，创建一个新的空图
            mappings_.clear();
            if (!SaveToFile(filename)) {
                std::cerr << "Failed to create empty dependency graph file: " << filename << std::endl;
                return false;
            }
            return true;
        }
        
        // 读取映射数量
        uint32_t count;
        ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
        if (!ifs.good()) {
            ifs.close();
            std::cerr << "Failed to read mapping count from: " << filename << std::endl;
            // 文件损坏，创建一个新的空图
            mappings_.clear();
            if (!SaveToFile(filename)) {
                std::cerr << "Failed to create empty dependency graph file: " << filename << std::endl;
                return false;
            }
            return true;
        }
        
        // 读取每个映射
        for (uint32_t i = 0; i < count; ++i) {
            BlockLocation loc;
            MappingEntry entry;
            
            ifs.read(reinterpret_cast<char*>(&loc.file_number), sizeof(loc.file_number));
            ifs.read(reinterpret_cast<char*>(&loc.block_id), sizeof(loc.block_id));
            ifs.read(reinterpret_cast<char*>(&entry.new_file_number), sizeof(entry.new_file_number));
            ifs.read(reinterpret_cast<char*>(&entry.new_block_range.start_block), sizeof(entry.new_block_range.start_block));
            ifs.read(reinterpret_cast<char*>(&entry.new_block_range.end_block), sizeof(entry.new_block_range.end_block));
            ifs.read(reinterpret_cast<char*>(&entry.ref_count), sizeof(entry.ref_count));
            
            if (!ifs.good()) {
                ifs.close();
                std::cerr << "Failed to read dependency graph entry at index " << i << " from: " << filename << std::endl;
                // 文件损坏，创建一个新的空图
                mappings_.clear();
                if (!SaveToFile(filename)) {
                    std::cerr << "Failed to create empty dependency graph file: " << filename << std::endl;
                    return false;
                }
                return true;
            }
            
            mappings_[loc] = entry;
        }
        
        ifs.close();
        return true;
    }
    
    // 统计信息
    size_t Size() const { return mappings_.size(); }
    
    bool Empty() const { return mappings_.empty(); }
    
    void PrintAll() const {
        std::cout << "\n=== Dependency Graph (Total: " << mappings_.size() << " mappings) ===" << std::endl;
        for (const auto& pair : mappings_) {
            const BlockLocation& loc = pair.first;
            const MappingEntry& entry = pair.second;
            std::cout << "  [File " << loc.file_number << ", Block " << loc.block_id << "] -> "
                     << "[File " << entry.new_file_number 
                     << ", Blocks " << entry.new_block_range.start_block 
                     << "-" << entry.new_block_range.end_block
                     << ", RefCount=" << entry.ref_count << "]" << std::endl;
        }
    }

    // 估算当前图占用的内存空间（仅包含映射表本身，不含 unordered_map 桶等额外开销）
    size_t ApproxMemoryUsageBytes() const {
        // 大致估算：每个元素包含一个 BlockLocation 和一个 MappingEntry
        const size_t per_entry =
            sizeof(BlockLocation) + sizeof(MappingEntry);
        return mappings_.size() * per_entry;
    }

    // 打印当前图占用的内存空间（近似值）
    void PrintMemoryUsage() const {
        size_t bytes = ApproxMemoryUsageBytes();
        double kb = static_cast<double>(bytes) / 1024.0;
        double mb = kb / 1024.0;

        std::cout << "\n=== Dependency Graph Memory Usage (Approx) ===" << std::endl;
        std::cout << "  Entries : " << mappings_.size() << std::endl;
        std::cout << "  Bytes   : " << bytes << " B" << std::endl;
        std::cout << "  KB      : " << kb << " KB" << std::endl;
        std::cout << "  MB      : " << mb << " MB" << std::endl;
    }
    
    void Clear() {
        mappings_.clear();
    }
    
    // 查找并打印完整的重定向路径（用于调试）
    void PrintRedirectPath(uint64_t file_number, uint32_t block_id) const {
        BlockLocation loc(file_number, block_id);
        std::cout << "Redirect path for [File " << file_number << ", Block " << block_id << "]: ";
        std::cout << "(" << loc.file_number << "," << loc.block_id << ")";
        
        int redirects = 0;
        const int max_redirect = 10;
        
        while (redirects < max_redirect) {
            auto it = mappings_.find(loc);
            if (it == mappings_.end()) {
                std::cout << " [FINAL]" << std::endl;
                return;
            }
            
            const MappingEntry& entry = it->second;
            uint32_t offset = loc.block_id - it->first.block_id;
            
            loc.file_number = entry.new_file_number;
            loc.block_id = entry.new_block_range.start_block + offset;
            
            std::cout << " -> (" << loc.file_number << "," << loc.block_id << ")";
            redirects++;
        }
        
        std::cout << " [MAX_DEPTH]" << std::endl;
    }

private:
    // 映射表：(origin_file, origin_block) -> (new_file, new_block_range, ref_count)
    std::unordered_map<BlockLocation, MappingEntry, BlockLocationHash> mappings_;
};

// 兼容旧接口的简单版本
struct Dependency {
    Dependency() {}

    uint64_t FindParent(uint64_t child) {
        if (dep_map_.find(child) == dep_map_.end()) {
            dep_map_[child] = child;
        }

        if (dep_map_[child] != child) {
            dep_map_[child] = FindParent(dep_map_[child]);
        }

        return dep_map_[child];
    }

    void SetParent(uint64_t parent, uint64_t child) {
        uint64_t root1 = FindParent(parent);
        uint64_t root2 = FindParent(child);

        if (root1 == root2) {
            return;
        }
        dep_map_[root2] = root1;
    }

    void PrintAll() {
        for (const auto& it : dep_map_) {
            std::cout << "Dep: " << it.first << " -> " <<  it.second << std::endl;
        }
    }

    std::unordered_map<uint64_t, uint64_t> dep_map_;
};

}

#endif  // DOUX_DEPENDENCY_H