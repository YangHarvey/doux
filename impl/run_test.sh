#!/bin/bash

# DependencyGraph 测试脚本
# 编译并运行测试

echo "========================================"
echo "  DependencyGraph 测试"
echo "========================================"

# 进入 impl 目录
cd "$(dirname "$0")"

# 编译测试
echo -e "\n[1/3] 编译测试程序..."
g++ -std=c++11 -I../include -I.. -o dependency_test dependency_test.cc

if [ $? -ne 0 ]; then
    echo "[ERROR] 编译失败"
    exit 1
fi

echo "[SUCCESS] 编译成功"

# 运行测试
echo -e "\n[2/3] 运行测试..."
./dependency_test

if [ $? -ne 0 ]; then
    echo -e "\n[ERROR] 测试失败"
    exit 1
fi

# 清理
echo -e "\n[3/3] 清理临时文件..."
rm -f /tmp/dependency_graph_test.bin
rm -f dependency_test

echo -e "\n[SUCCESS] 所有测试通过并清理完成！"
