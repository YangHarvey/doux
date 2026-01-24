// DependencyGraph 功能测试
// 编译: g++ -std=c++11 -I../include -I.. dependency_test.cc -o dependency_test
// 运行: ./dependency_test

#include "dependency.h"
#include <iostream>
#include <cassert>
#include <iomanip>

using namespace doux;

// 辅助函数：打印测试标题
void PrintTestHeader(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

// 辅助函数：打印通过/失败
void PrintResult(bool passed, const std::string& test_name) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << test_name << std::endl;
}

// 测试 1: 基本的单个 block 映射
void Test1_BasicMapping() {
    PrintTestHeader("测试 1: 基本的单个 Block 映射");
    
    DependencyGraph dep_graph;
    
    // 添加单个 block 映射
    dep_graph.AddMapping(100, BlockRange(5, 5), 200, BlockRange(10, 10));
    
    uint64_t new_file;
    uint32_t new_block;
    
    // 查找应该找到映射
    bool found = dep_graph.LookupBlock(100, 5, &new_file, &new_block);
    
    std::cout << "原始位置: File 100, Block 5" << std::endl;
    std::cout << "映射到: File " << new_file << ", Block " << new_block << std::endl;
    std::cout << "是否重定向: " << (found ? "是" : "否") << std::endl;
    
    PrintResult(found && new_file == 200 && new_block == 10, "单个 block 映射查找");
    
    // 查找不存在的映射
    found = dep_graph.LookupBlock(100, 6, &new_file, &new_block);
    PrintResult(!found && new_file == 100 && new_block == 6, "查找不存在的映射");
}

// 测试 2: Block Range 映射
void Test2_RangeMapping() {
    PrintTestHeader("测试 2: Block Range 批量映射");
    
    DependencyGraph dep_graph;
    
    // 添加 range 映射: File 100 的 blocks 0-9 -> File 200 的 blocks 10-19
    BlockRange origin_range(0, 9);
    BlockRange new_range(10, 19);
    dep_graph.AddRangeMapping(100, origin_range, 200, new_range);
    
    std::cout << "映射: File 100, Blocks [0-9] -> File 200, Blocks [10-19]" << std::endl;
    std::cout << "\n查找结果:" << std::endl;
    
    bool all_passed = true;
    for (uint32_t i = 0; i < 10; ++i) {
        uint64_t new_file;
        uint32_t new_block;
        dep_graph.LookupBlock(100, i, &new_file, &new_block);
        
        std::cout << "  Block " << i << " -> File " << new_file 
                  << ", Block " << new_block << std::endl;
        
        if (new_file != 200 || new_block != 10 + i) {
            all_passed = false;
        }
    }
    
    PrintResult(all_passed, "Range 映射的所有 blocks 都正确");
}

// 测试 3: 级联查找 (A->B->C)
void Test3_CascadingLookup() {
    PrintTestHeader("测试 3: 级联查找 (多层重定向)");
    
    DependencyGraph dep_graph;
    
    // 第一层: File 100 -> File 200
    dep_graph.AddRangeMapping(100, BlockRange(0, 9), 200, BlockRange(0, 9));
    std::cout << "第 1 次 Compaction: File 100 [0-9] -> File 200 [0-9]" << std::endl;
    
    // 第二层: File 200 的部分 blocks -> File 300
    dep_graph.AddRangeMapping(200, BlockRange(0, 4), 300, BlockRange(10, 14));
    std::cout << "第 2 次 Compaction: File 200 [0-4] -> File 300 [10-14]" << std::endl;
    
    // 第三层: File 300 的部分 blocks -> File 400
    dep_graph.AddRangeMapping(300, BlockRange(10, 12), 400, BlockRange(20, 22));
    std::cout << "第 3 次 Compaction: File 300 [10-12] -> File 400 [20-22]" << std::endl;
    
    std::cout << "\n=== 级联查找结果 ===" << std::endl;
    
    struct TestCase {
        uint64_t file;
        uint32_t block;
        uint64_t expected_file;
        uint32_t expected_block;
        int expected_redirects;
        std::string description;
    };
    
    TestCase cases[] = {
        {100, 0, 400, 20, 3, "三层重定向: 100->200->300->400"},
        {100, 1, 400, 21, 3, "三层重定向: 100->200->300->400"},
        {100, 2, 400, 22, 3, "三层重定向: 100->200->300->400"},
        {100, 3, 300, 13, 2, "两层重定向: 100->200->300"},
        {100, 4, 300, 14, 2, "两层重定向: 100->200->300"},
        {100, 5, 200, 5, 1, "一层重定向: 100->200"},
        {100, 9, 200, 9, 1, "一层重定向: 100->200"},
        {200, 0, 400, 20, 2, "两层重定向: 200->300->400"},
        {300, 10, 400, 20, 1, "一层重定向: 300->400"},
        {400, 20, 400, 20, 0, "无重定向（最终位置）"},
    };
    
    bool all_passed = true;
    for (const auto& tc : cases) {
        uint64_t new_file;
        uint32_t new_block;
        int redirect_count = 0;
        
        dep_graph.LookupBlock(tc.file, tc.block, &new_file, &new_block, &redirect_count);
        
        bool passed = (new_file == tc.expected_file && 
                      new_block == tc.expected_block &&
                      redirect_count == tc.expected_redirects);
        
        std::cout << std::setw(15) << std::left 
                  << ("File " + std::to_string(tc.file) + ", Block " + std::to_string(tc.block))
                  << " -> File " << new_file << ", Block " << new_block
                  << " (" << redirect_count << " 层) - " << tc.description
                  << (passed ? " ✓" : " ✗") << std::endl;
        
        if (!passed) all_passed = false;
    }
    
    std::cout << "\n=== 可视化重定向路径 ===" << std::endl;
    dep_graph.PrintRedirectPath(100, 0);
    dep_graph.PrintRedirectPath(100, 3);
    dep_graph.PrintRedirectPath(100, 5);
    dep_graph.PrintRedirectPath(200, 0);
    dep_graph.PrintRedirectPath(300, 10);
    
    PrintResult(all_passed, "所有级联查找测试");
}

// 测试 4: 引用计数和垃圾回收
void Test4_RefCountAndGC() {
    PrintTestHeader("测试 4: 引用计数和垃圾回收");
    
    DependencyGraph dep_graph;
    
    // 添加映射
    dep_graph.AddRangeMapping(100, BlockRange(0, 9), 200, BlockRange(0, 9));
    dep_graph.AddRangeMapping(200, BlockRange(0, 4), 300, BlockRange(10, 14));
    
    size_t initial_size = dep_graph.Size();
    std::cout << "初始映射数量: " << initial_size << std::endl;
    
    // 减少部分引用计数
    std::cout << "\n减少 File 100, Blocks [0-4] 的引用计数..." << std::endl;
    dep_graph.DecrementRangeRef(100, BlockRange(0, 4));
    
    // 执行垃圾回收
    size_t removed = dep_graph.GarbageCollect();
    std::cout << "垃圾回收移除了 " << removed << " 个映射" << std::endl;
    std::cout << "剩余映射数量: " << dep_graph.Size() << std::endl;
    
    PrintResult(removed == 5 && dep_graph.Size() == initial_size - 5, 
                "引用计数和垃圾回收");
}

// 测试 5: 持久化和恢复
void Test5_Persistence() {
    PrintTestHeader("测试 5: 持久化和恢复");
    
    const std::string filename = "/tmp/dependency_graph_test.bin";
    
    // 创建原始 graph
    DependencyGraph original_graph;
    original_graph.AddRangeMapping(100, BlockRange(0, 9), 200, BlockRange(0, 9));
    original_graph.AddRangeMapping(200, BlockRange(0, 4), 300, BlockRange(10, 14));
    
    size_t original_size = original_graph.Size();
    std::cout << "原始 graph 映射数量: " << original_size << std::endl;
    
    // 保存到文件
    bool s = original_graph.SaveToFile(filename);
    PrintResult(s, "保存到文件");
    
    // 从文件恢复
    DependencyGraph recovered_graph;
    s = recovered_graph.LoadFromFile(filename);
    PrintResult(s, "从文件恢复");
    
    size_t recovered_size = recovered_graph.Size();
    std::cout << "恢复的 graph 映射数量: " << recovered_size << std::endl;
    
    // 验证数据一致性
    bool data_consistent = (original_size == recovered_size);
    
    // 测试几个查询
    uint64_t new_file1, new_file2;
    uint32_t new_block1, new_block2;
    
    original_graph.LookupBlock(100, 3, &new_file1, &new_block1);
    recovered_graph.LookupBlock(100, 3, &new_file2, &new_block2);
    
    data_consistent = data_consistent && 
                     (new_file1 == new_file2) && 
                     (new_block1 == new_block2);
    
    PrintResult(data_consistent, "持久化数据一致性");
}

// 测试 6: 复杂场景 - 模拟真实的 VCompaction
void Test6_RealWorldScenario() {
    PrintTestHeader("测试 6: 真实场景模拟");
    
    DependencyGraph dep_graph;
    
    std::cout << "=== 模拟多次 VCompaction ===" << std::endl;
    
    // 初始状态: 3 个 L0 文件
    std::cout << "\n初始状态:" << std::endl;
    std::cout << "  L0: File 100 (100 blocks), File 101 (80 blocks), File 102 (120 blocks)" << std::endl;
    
    // Compaction 1: L0 -> L1
    std::cout << "\nCompaction 1: L0 -> L1" << std::endl;
    std::cout << "  File 100 [0-99]   -> File 200 [0-99]" << std::endl;
    std::cout << "  File 101 [0-79]   -> File 200 [100-179]" << std::endl;
    std::cout << "  File 102 [0-119]  -> File 200 [180-299]" << std::endl;
    
    dep_graph.AddRangeMapping(100, BlockRange(0, 99), 200, BlockRange(0, 99));
    dep_graph.AddRangeMapping(101, BlockRange(0, 79), 200, BlockRange(100, 179));
    dep_graph.AddRangeMapping(102, BlockRange(0, 119), 200, BlockRange(180, 299));
    
    std::cout << "  映射数量: " << dep_graph.Size() << std::endl;
    
    // Compaction 2: L1 部分数据 -> L2
    std::cout << "\nCompaction 2: L1 -> L2" << std::endl;
    std::cout << "  File 200 [0-149]  -> File 300 [0-149]" << std::endl;
    
    dep_graph.AddRangeMapping(200, BlockRange(0, 149), 300, BlockRange(0, 149));
    std::cout << "  映射数量: " << dep_graph.Size() << std::endl;
    
    // 测试查询
    std::cout << "\n=== 查询不同来源的 blocks ===" << std::endl;
    
    struct Query {
        uint64_t file;
        uint32_t block;
        std::string description;
    };
    
    Query queries[] = {
        {100, 50, "来自原始 File 100 (经过 2 层重定向)"},
        {101, 40, "来自原始 File 101 (经过 2 层重定向)"},
        {102, 60, "来自原始 File 102 (经过 1 层重定向)"},
        {200, 200, "来自 L1 File 200 (未被进一步 compact)"},
    };
    
    for (const auto& q : queries) {
        uint64_t new_file;
        uint32_t new_block;
        int redirect_count = 0;
        
        dep_graph.LookupBlock(q.file, q.block, &new_file, &new_block, &redirect_count);
        
        std::cout << "  查询 File " << q.file << ", Block " << q.block << ":" << std::endl;
        std::cout << "    -> 最终位置: File " << new_file << ", Block " << new_block << std::endl;
        std::cout << "    -> 重定向层数: " << redirect_count << std::endl;
        std::cout << "    -> 说明: " << q.description << std::endl;
        std::cout << "    -> 完整路径: ";
        dep_graph.PrintRedirectPath(q.file, q.block);
    }
    
    // 展示能查找到的信息级别
    std::cout << "\n=== 通过 Block ID 能查找到的信息 ===" << std::endl;
    std::cout << "给定输入: SSTable File Number + Block ID" << std::endl;
    std::cout << "能查找到的信息:" << std::endl;
    std::cout << "  1. ✓ 当前最终的 File Number (新的 VSST)" << std::endl;
    std::cout << "  2. ✓ 当前最终的 Block ID (在新 VSST 中的位置)" << std::endl;
    std::cout << "  3. ✓ 重定向层数 (经过了多少次 compaction)" << std::endl;
    std::cout << "  4. ✓ 完整的重定向路径 (调试用)" << std::endl;
    std::cout << "  5. ✓ 引用计数 (有多少 vptr 还在使用这个映射)" << std::endl;
    std::cout << "\n配合其他信息可以得到:" << std::endl;
    std::cout << "  - Block 在新文件中的物理偏移 (通过 block_id * block_size)" << std::endl;
    std::cout << "  - 具体的 value 数据 (通过 InternalVGet 读取)" << std::endl;
    std::cout << "  - Block 内的 offset (如果还记录了 block_offset)" << std::endl;
}

// 测试 7: 边界情况
void Test7_EdgeCases() {
    PrintTestHeader("测试 7: 边界情况");
    
    DependencyGraph dep_graph;
    
    // 测试空 graph
    uint64_t new_file;
    uint32_t new_block;
    bool found = dep_graph.LookupBlock(100, 0, &new_file, &new_block);
    PrintResult(!found && new_file == 100 && new_block == 0, "空 graph 查找");
    
    // 测试单个 block 的 range
    dep_graph.AddRangeMapping(100, BlockRange(5, 5), 200, BlockRange(10, 10));
    found = dep_graph.LookupBlock(100, 5, &new_file, &new_block);
    PrintResult(found && new_file == 200 && new_block == 10, "单个 block 的 range");
    
    // 测试相同 file 不同 ranges
    dep_graph.AddRangeMapping(100, BlockRange(0, 4), 200, BlockRange(0, 4));
    dep_graph.AddRangeMapping(100, BlockRange(10, 14), 200, BlockRange(20, 24));
    
    found = dep_graph.LookupBlock(100, 2, &new_file, &new_block);
    bool test1 = (found && new_file == 200 && new_block == 2);
    
    found = dep_graph.LookupBlock(100, 7, &new_file, &new_block);
    bool test2 = (!found && new_file == 100 && new_block == 7);  // gap 中间
    
    found = dep_graph.LookupBlock(100, 12, &new_file, &new_block);
    bool test3 = (found && new_file == 200 && new_block == 22);
    
    PrintResult(test1 && test2 && test3, "不连续的 ranges");
}

// 主测试函数
int main() {
    std::cout << "\n" << std::string(70, '#') << std::endl;
    std::cout << "#" << std::string(68, ' ') << "#" << std::endl;
    std::cout << "#" << "       DependencyGraph 功能测试套件" << std::string(30, ' ') << "#" << std::endl;
    std::cout << "#" << std::string(68, ' ') << "#" << std::endl;
    std::cout << std::string(70, '#') << std::endl;
    
    try {
        Test1_BasicMapping();
        Test2_RangeMapping();
        Test3_CascadingLookup();
        Test4_RefCountAndGC();
        Test5_Persistence();
        Test6_RealWorldScenario();
        Test7_EdgeCases();
        
        std::cout << "\n" << std::string(70, '#') << std::endl;
        std::cout << "#" << std::string(68, ' ') << "#" << std::endl;
        std::cout << "#" << "       所有测试完成！" << std::string(44, ' ') << "#" << std::endl;
        std::cout << "#" << std::string(68, ' ') << "#" << std::endl;
        std::cout << std::string(70, '#') << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] 测试失败: " << e.what() << std::endl;
        return 1;
    }
}
