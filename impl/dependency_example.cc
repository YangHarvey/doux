// 示例：如何使用新的 DependencyGraph
#include "dependency.h"
#include <iostream>

using namespace doux;

int main() {
    DependencyGraph dep_graph;
    
    // 场景 1：VCompaction 将 File 100 的 Block 0-9 迁移到 File 200 的 Block 0-9
    std::cout << "=== 场景 1: 简单迁移 ===" << std::endl;
    BlockRange origin_range1(0, 9);   // File 100: blocks 0-9
    BlockRange new_range1(0, 9);      // File 200: blocks 0-9
    dep_graph.AddRangeMapping(100, origin_range1, 200, new_range1);
    
    // 查找 File 100, Block 5 的新位置
    uint64_t new_file;
    uint32_t new_block;
    if (dep_graph.LookupBlock(100, 5, &new_file, &new_block)) {
        std::cout << "File 100, Block 5 -> File " << new_file 
                  << ", Block " << new_block << std::endl;
    }
    
    // 场景 2：File 100 的某些 blocks 再次被迁移到 File 300（级联迁移）
    std::cout << "\n=== 场景 2: 级联迁移 (A->B->C) ===" << std::endl;
    BlockRange origin_range2(0, 4);   // File 200: blocks 0-4 (来自 File 100)
    BlockRange new_range2(10, 14);    // File 300: blocks 10-14
    dep_graph.AddRangeMapping(200, origin_range2, 300, new_range2);
    
    // 查找 File 100, Block 2 的新位置（会经过两层重定向：100->200->300）
    int redirect_count = 0;
    if (dep_graph.LookupBlock(100, 2, &new_file, &new_block, &redirect_count)) {
        std::cout << "File 100, Block 2 -> File " << new_file 
                  << ", Block " << new_block 
                  << " (经过 " << redirect_count << " 层重定向)" << std::endl;
    }
    
    // 场景 2.5：继续迁移 File 300 的部分 blocks 到 File 400（三层级联）
    std::cout << "\n=== 场景 2.5: 三层级联 (A->B->C->D) ===" << std::endl;
    BlockRange origin_range3(10, 12);  // File 300: blocks 10-12 (来自 File 200)
    BlockRange new_range3(20, 22);     // File 400: blocks 20-22
    dep_graph.AddRangeMapping(300, origin_range3, 400, new_range3);
    
    // 查找 File 100, Block 0 的最终位置（会经过三层重定向：100->200->300->400）
    if (dep_graph.LookupBlock(100, 0, &new_file, &new_block, &redirect_count)) {
        std::cout << "File 100, Block 0 -> File " << new_file 
                  << ", Block " << new_block 
                  << " (经过 " << redirect_count << " 层重定向)" << std::endl;
    }
    
    // 对比：File 100, Block 5 只经过一层重定向（100->200）
    if (dep_graph.LookupBlock(100, 5, &new_file, &new_block, &redirect_count)) {
        std::cout << "File 100, Block 5 -> File " << new_file 
                  << ", Block " << new_block 
                  << " (经过 " << redirect_count << " 层重定向)" << std::endl;
    }
    
    // 可视化重定向路径
    std::cout << "\n=== 可视化重定向路径 ===" << std::endl;
    dep_graph.PrintRedirectPath(100, 0);  // 三层：100->200->300->400
    dep_graph.PrintRedirectPath(100, 2);  // 两层：100->200->300
    dep_graph.PrintRedirectPath(100, 5);  // 一层：100->200
    dep_graph.PrintRedirectPath(100, 9);  // 一层：100->200
    dep_graph.PrintRedirectPath(200, 0);  // 两层：200->300->400
    dep_graph.PrintRedirectPath(300, 10); // 一层：300->400
    dep_graph.PrintRedirectPath(500, 0);  // 无映射
    
    // 场景 3：打印当前所有映射
    std::cout << "\n=== 场景 3: 当前所有映射 ===" << std::endl;
    dep_graph.PrintAll();
    
    // 场景 4：持久化到磁盘
    std::cout << "\n=== 场景 4: 持久化 ===" << std::endl;
    auto s = dep_graph.SaveToFile("/tmp/dependency_graph.bin");
    if (s.ok()) {
        std::cout << "成功保存到文件，共 " << dep_graph.Size() << " 个映射" << std::endl;
    }
    
    // 场景 5：从磁盘恢复
    DependencyGraph new_graph;
    s = new_graph.LoadFromFile("/tmp/dependency_graph.bin");
    if (s.ok()) {
        std::cout << "成功从文件恢复，共 " << new_graph.Size() << " 个映射" << std::endl;
    }
    
    // 场景 6：引用计数和垃圾回收
    std::cout << "\n=== 场景 6: 引用计数管理 ===" << std::endl;
    std::cout << "当前映射数量: " << dep_graph.Size() << std::endl;
    
    // 模拟：KO-tree 中的 vptr 被更新，旧的映射不再需要
    dep_graph.DecrementRangeRef(100, BlockRange(0, 9));
    std::cout << "减少引用计数后..." << std::endl;
    
    // 执行垃圾回收
    size_t removed = dep_graph.GarbageCollect();
    std::cout << "垃圾回收移除了 " << removed << " 个映射" << std::endl;
    std::cout << "剩余映射数量: " << dep_graph.Size() << std::endl;
    
    return 0;
}

/* 预期输出：
=== 场景 1: 简单迁移 ===
File 100, Block 5 -> File 200, Block 5

=== 场景 2: 级联迁移 (A->B->C) ===
File 100, Block 2 -> File 300, Block 12 (经过 2 层重定向)

=== 场景 2.5: 三层级联 (A->B->C->D) ===
File 100, Block 0 -> File 400, Block 20 (经过 3 层重定向)
File 100, Block 5 -> File 200, Block 5 (经过 1 层重定向)

=== 可视化重定向路径 ===
Redirect path for [File 100, Block 0]: (100,0) -> (200,0) -> (300,10) -> (400,20) [FINAL]
Redirect path for [File 100, Block 2]: (100,2) -> (200,2) -> (300,12) [FINAL]
Redirect path for [File 100, Block 5]: (100,5) -> (200,5) [FINAL]
Redirect path for [File 100, Block 9]: (100,9) -> (200,9) [FINAL]
Redirect path for [File 200, Block 0]: (200,0) -> (300,10) -> (400,20) [FINAL]
Redirect path for [File 300, Block 10]: (300,10) -> (400,20) [FINAL]
Redirect path for [File 500, Block 0]: (500,0) [FINAL]

=== 场景 3: 当前所有映射 ===

=== Dependency Graph (Total: 18 mappings) ===
  [File 100, Block 0] -> [File 200, Blocks 0-0, RefCount=1]
  [File 100, Block 1] -> [File 200, Blocks 1-1, RefCount=1]
  ...
  [File 200, Block 0] -> [File 300, Blocks 10-10, RefCount=1]
  ...
  [File 300, Block 10] -> [File 400, Blocks 20-20, RefCount=1]
  ...

=== 场景 4: 持久化 ===
成功保存到文件，共 18 个映射

=== 场景 5: 从磁盘恢复 ===
成功从文件恢复，共 18 个映射

=== 场景 6: 引用计数管理 ===
当前映射数量: 18
减少引用计数后...
垃圾回收移除了 10 个映射
剩余映射数量: 8
*/
