#!/bin/bash

# 写放大测量脚本
# 用法: 
#   1. 使用默认参数: ./measure_write_amplification.sh
#   2. 传入完整命令: ./measure_write_amplification.sh -m 10 -f "/path/to/file" -c 60000000 -d "/path/to/db"
#   3. 只传入数据库目录（其他使用默认值）: ./measure_write_amplification.sh -d "/path/to/db"

# 检查 bc 命令是否可用
if ! command -v bc &> /dev/null; then
    echo "错误: 需要安装 bc 命令来计算数值"
    echo "请运行: sudo yum install bc 或 sudo apt-get install bc"
    exit 1
fi

# 查找 tpch_bench_load_data 可执行文件
# 优先查找顺序: 1) 当前目录 2) ../build 3) ../../build
EXECUTABLE=""
if [ -f "./tpch_bench_load_data" ]; then
    EXECUTABLE="./tpch_bench_load_data"
elif [ -f "../build/tpch_bench_load_data" ]; then
    EXECUTABLE="../build/tpch_bench_load_data"
elif [ -f "../../build/tpch_bench_load_data" ]; then
    EXECUTABLE="../../build/tpch_bench_load_data"
else
    echo "错误: 找不到 tpch_bench_load_data 可执行文件"
    echo "请确保:"
    echo "  1. 在 impl 目录下运行此脚本，或"
    echo "  2. 使用 -b 参数指定 build 目录路径"
    echo ""
    echo "尝试查找的位置:"
    echo "  - ./tpch_bench_load_data"
    echo "  - ../build/tpch_bench_load_data"
    echo "  - ../../build/tpch_bench_load_data"
    exit 1
fi

# 默认参数
MOD=10
INPUT_FILE="/home/ysm/work/tpch-data/lineitem.10"
DATA_COUNT=60000000
DB_DIR="/home/ysm/work/test/doux_db_10"
BUILD_DIR=""

# 如果提供了参数，使用提供的参数
if [ $# -gt 0 ]; then
    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -m|--modification)
                MOD="$2"
                shift 2
                ;;
            -f|--input_file)
                INPUT_FILE="$2"
                shift 2
                ;;
            -c|--count)
                DATA_COUNT="$2"
                shift 2
                ;;
            -d|--directory)
                DB_DIR="$2"
                shift 2
                ;;
            -b|--build_dir)
                BUILD_DIR="$2"
                shift 2
                ;;
            *)
                echo "未知参数: $1"
                echo "用法: $0 [-m MOD] [-f INPUT_FILE] [-c COUNT] [-d DB_DIR] [-b BUILD_DIR]"
                exit 1
                ;;
        esac
    done
fi

# 如果指定了 build 目录，使用指定的路径
if [ -n "$BUILD_DIR" ]; then
    if [ -f "$BUILD_DIR/tpch_bench_load_data" ]; then
        EXECUTABLE="$BUILD_DIR/tpch_bench_load_data"
    else
        echo "错误: 在指定的 build 目录中找不到 tpch_bench_load_data"
        echo "  指定路径: $BUILD_DIR"
        exit 1
    fi
fi

# 从代码中获取 key_size 和 value_size
# 根据 tpch_bench_load_data.cc，key_size = 16, value_size = 314
KEY_SIZE=16
VALUE_SIZE=314

echo "=========================================="
echo "写放大测量脚本"
echo "=========================================="
echo "参数配置:"
echo "  可执行文件: $EXECUTABLE"
if [ "$MOD" = "0" ]; then
    echo "  MOD: $MOD (LevelDB)"
elif [ "$MOD" = "10" ]; then
    echo "  MOD: $MOD (Doux)"
else
    echo "  MOD: $MOD"
fi
echo "  输入文件: $INPUT_FILE"
echo "  数据条数: $DATA_COUNT"
echo "  数据库目录: $DB_DIR"
echo "  Key大小: $KEY_SIZE 字节"
echo "  Value大小: $VALUE_SIZE 字节"
echo "=========================================="
echo ""

# 计算逻辑写入量（每个 Put 操作的 key + value 大小）
# 注意: 这里只计算用户数据，不包括 LevelDB 的内部开销（如索引、元数据等）
LOGICAL_WRITE_SIZE=$(( ($KEY_SIZE + $VALUE_SIZE) * $DATA_COUNT ))
LOGICAL_WRITE_MB=$(echo "scale=2; $LOGICAL_WRITE_SIZE / 1024 / 1024" | bc)
LOGICAL_WRITE_GB=$(echo "scale=2; $LOGICAL_WRITE_SIZE / 1024 / 1024 / 1024" | bc)

echo "逻辑写入量:"
echo "  $LOGICAL_WRITE_SIZE 字节"
echo "  $LOGICAL_WRITE_MB MB"
echo "  $LOGICAL_WRITE_GB GB"
echo ""

# 删除旧的数据库目录（如果存在）
BEFORE_SIZE=0
if [ -d "$DB_DIR" ]; then
    BEFORE_SIZE=$(du -sb "$DB_DIR" 2>/dev/null | cut -f1)
    if [ -n "$BEFORE_SIZE" ] && [ "$BEFORE_SIZE" != "0" ]; then
        BEFORE_SIZE_MB=$(echo "scale=2; $BEFORE_SIZE / 1024 / 1024" | bc)
        BEFORE_SIZE_GB=$(echo "scale=2; $BEFORE_SIZE / 1024 / 1024 / 1024" | bc)
        echo "检测到已存在的数据库目录:"
        echo "  大小: $BEFORE_SIZE 字节 ($BEFORE_SIZE_MB MB, $BEFORE_SIZE_GB GB)"
        echo "  正在删除旧数据..."
        rm -rf "$DB_DIR"
        if [ $? -eq 0 ]; then
            echo "  ✓ 旧数据已删除"
        else
            echo "  ✗ 删除失败，请手动删除: $DB_DIR"
            exit 1
        fi
    else
        echo "数据库目录存在但为空"
    fi
    echo ""
else
    echo "数据库目录不存在，将创建新目录"
    echo ""
fi

# 运行 load_data 命令并捕获输出
echo "开始执行 load_data..."
echo "可执行文件: $EXECUTABLE"
echo "命令: $EXECUTABLE -m $MOD -f \"$INPUT_FILE\" -c $DATA_COUNT -d \"$DB_DIR\""
echo ""

# 检查是否使用系统级统计（strace）
USE_STRACE=false
if command -v strace &> /dev/null; then
    USE_STRACE=true
    echo "使用 strace 跟踪系统调用以获取真实的磁盘写入量"
    echo ""
fi

START_TIME=$(date +%s)
# 捕获输出到临时文件，同时显示在终端
TEMP_OUTPUT=$(mktemp)
STRACE_OUTPUT=""

if [ "$USE_STRACE" = true ]; then
    # 使用 strace 跟踪 write/pwrite 系统调用
    STRACE_OUTPUT=$(mktemp)
    strace -e trace=write,pwrite64,writev,pwritev -f -o "$STRACE_OUTPUT" \
        "$EXECUTABLE" -m $MOD -f "$INPUT_FILE" -c $DATA_COUNT -d "$DB_DIR" 2>&1 | tee "$TEMP_OUTPUT"
    LOAD_STATUS=${PIPESTATUS[0]}
else
    "$EXECUTABLE" -m $MOD -f "$INPUT_FILE" -c $DATA_COUNT -d "$DB_DIR" 2>&1 | tee "$TEMP_OUTPUT"
    LOAD_STATUS=${PIPESTATUS[0]}
fi

END_TIME=$(date +%s)
ELAPSED_TIME=$((END_TIME - START_TIME))

echo ""
if [ $LOAD_STATUS -ne 0 ]; then
    echo "错误: load_data 执行失败，退出码: $LOAD_STATUS"
    rm -f "$TEMP_OUTPUT"
    exit 1
fi

echo "load_data 执行完成，耗时: ${ELAPSED_TIME} 秒"
echo ""

# 从输出中解析总写入量（进程全程写入的磁盘数据量）
TOTAL_BYTES_WRITTEN=0
TOTAL_BYTES_WRITTEN_STRACE=0

# 方法1: 从程序内部统计获取
if grep -q "STATS_TOTAL_BYTES_WRITTEN=" "$TEMP_OUTPUT"; then
    TOTAL_BYTES_WRITTEN=$(grep "STATS_TOTAL_BYTES_WRITTEN=" "$TEMP_OUTPUT" | sed 's/.*STATS_TOTAL_BYTES_WRITTEN=//' | tr -d '\r\n')
fi

# 方法2: 从 strace 输出获取真实的系统调用写入量（更客观）
if [ "$USE_STRACE" = true ] && [ -f "$STRACE_OUTPUT" ]; then
    echo "=========================================="
    echo "系统级写入统计（strace 跟踪）"
    echo "=========================================="
    # 解析 strace 输出，累加所有 write/pwrite 系统调用的写入字节数
    # strace 格式: write(3, "data", 1024) = 1024
    # 或者: pwrite64(3, "data", 1024, 4096) = 1024
    # 或者: [pid 12345] write(3, "data", 1024) = 1024
    # 或者: writev(3, [{iov_base="data", iov_len=1024}], 1) = 1024
    # 需要过滤掉错误返回值（负数）和特殊值（如 -1）
    # 使用 bc 累加，避免 awk 的 32 位整数溢出问题
    STRACE_TEMP=$(mktemp)
    grep -E "write\(|pwrite|writev" "$STRACE_OUTPUT" | \
        grep -E "= [0-9]+$" | \
        grep -vE "= -[0-9]+$" | \
        sed -E 's/.*= ([0-9]+)$/\1/' | \
        grep -E '^[0-9]+$' > "$STRACE_TEMP"
    
    # 使用 bc 累加所有数字，避免整数溢出
    if [ -s "$STRACE_TEMP" ]; then
        # 方法：将所有数字用 + 连接，最后加上 0，然后传给 bc
        # 例如: echo "1+2+3+0" | bc
        TOTAL_BYTES_WRITTEN_STRACE=$(paste -sd+ "$STRACE_TEMP" | sed 's/$/+0/' | bc)
        # 确保输出是整数格式（去掉小数点）
        if [ -n "$TOTAL_BYTES_WRITTEN_STRACE" ]; then
            TOTAL_BYTES_WRITTEN_STRACE=$(echo "$TOTAL_BYTES_WRITTEN_STRACE" | awk '{printf "%.0f", $1}')
        else
            TOTAL_BYTES_WRITTEN_STRACE=0
        fi
    else
        TOTAL_BYTES_WRITTEN_STRACE=0
    fi
    rm -f "$STRACE_TEMP"
    
    # 使用 bc 来比较，避免科学计数法问题
    if [ -n "$TOTAL_BYTES_WRITTEN_STRACE" ] && [ "$TOTAL_BYTES_WRITTEN_STRACE" != "0" ] && \
       (( $(echo "$TOTAL_BYTES_WRITTEN_STRACE > 0" | bc -l) )); then
        STRACE_MB=$(echo "scale=2; $TOTAL_BYTES_WRITTEN_STRACE / 1024 / 1024" | bc)
        STRACE_GB=$(echo "scale=2; $TOTAL_BYTES_WRITTEN_STRACE / 1024 / 1024 / 1024" | bc)
        echo "strace 统计的总写入量: $TOTAL_BYTES_WRITTEN_STRACE 字节 ($STRACE_MB MB, $STRACE_GB GB)"
        echo "说明: 这是通过跟踪所有 write/pwrite/writev 系统调用得到的真实磁盘写入量"
        echo "     包括所有文件写入操作，比程序内部统计更全面"
        echo ""
        
        # 如果两种方法差异很大，给出警告
        if [ $TOTAL_BYTES_WRITTEN -gt 0 ]; then
            DIFF_PERCENT=$(echo "scale=2; ($TOTAL_BYTES_WRITTEN_STRACE - $TOTAL_BYTES_WRITTEN) * 100 / $TOTAL_BYTES_WRITTEN" | bc)
            ABS_DIFF_PERCENT=$(echo "scale=2; if ($DIFF_PERCENT < 0) -($DIFF_PERCENT) else $DIFF_PERCENT" | bc)
            if (( $(echo "$ABS_DIFF_PERCENT > 10" | bc -l) )); then
                echo "⚠ 警告: 系统级统计与程序内部统计差异较大: ${DIFF_PERCENT}%"
                echo "  程序内部统计: $TOTAL_BYTES_WRITTEN 字节 ($(echo "scale=2; $TOTAL_BYTES_WRITTEN / 1024 / 1024" | bc) MB)"
                echo "  系统级统计: $TOTAL_BYTES_WRITTEN_STRACE 字节 ($STRACE_MB MB)"
                echo "  建议使用系统级统计（更准确，包含所有写入操作）"
                echo ""
            else
                echo "✓ 系统级统计与程序内部统计基本一致（差异 < 10%）"
                echo ""
            fi
        fi
        
        # 使用系统级统计作为主要数据源（更准确）
        TOTAL_BYTES_WRITTEN=$TOTAL_BYTES_WRITTEN_STRACE
    else
        echo "无法从 strace 输出中解析写入量，将使用程序内部统计"
        echo "（可能 strace 输出格式不匹配，或没有检测到写入操作）"
        echo ""
        rm -f "$STRACE_OUTPUT"
    fi
fi

# 解析每个 level 的写入统计（用于分析写放大来源）
echo "=========================================="
echo "Per-Level 写入统计（用于分析写放大来源）"
echo "=========================================="
if grep -q "Per-Level Write Breakdown" "$TEMP_OUTPUT"; then
    # 提取统计信息部分（从 "Per-Level Write Breakdown" 到下一个 "===" 或文件结尾）
    awk '/Per-Level Write Breakdown/{flag=1; next} /^=== Detailed Write Statistics/{if(flag) exit} flag' "$TEMP_OUTPUT"
    echo ""
fi

# 提取详细统计信息
if grep -q "Detailed Write Statistics by Level" "$TEMP_OUTPUT"; then
    echo "=========================================="
    echo "详细写入统计（按 Level 分类）"
    echo "=========================================="
    awk '/Detailed Write Statistics by Level/{flag=1; next} /^===/{if(flag) exit} flag' "$TEMP_OUTPUT"
    echo ""
    
    # 提取 Level 0, 1, 2 的写入量和 compaction 次数
    LEVEL0_WRITE=$(grep "Level 0 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ bytes' | head -1 | grep -oE '[0-9]+' || echo "0")
    LEVEL1_WRITE=$(grep "Level 1 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ bytes' | head -1 | grep -oE '[0-9]+' || echo "0")
    LEVEL2_WRITE=$(grep "Level 2 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ bytes' | head -1 | grep -oE '[0-9]+' || echo "0")
    
    # 提取 compaction 次数
    LEVEL0_COUNT=$(grep "Level 0 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ operations' | head -1 | grep -oE '[0-9]+' || echo "0")
    LEVEL1_COUNT=$(grep "Level 1 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ compactions' | head -1 | grep -oE '[0-9]+' || echo "0")
    LEVEL2_COUNT=$(grep "Level 2 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+ compactions' | head -1 | grep -oE '[0-9]+' || echo "0")
    TOTAL_COMPACTION_COUNT=$(grep "Total compaction count:" "$TEMP_OUTPUT" | grep -oE '[0-9]+' || echo "0")
    
    # 提取每次 compaction 的平均数据量
    LEVEL0_AVG=$(grep "Level 0 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+\.[0-9]+ MB per operation' | head -1 | grep -oE '[0-9]+\.[0-9]+' || echo "0")
    LEVEL1_AVG=$(grep "Level 1 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+\.[0-9]+ MB per compaction' | head -1 | grep -oE '[0-9]+\.[0-9]+' || echo "0")
    LEVEL2_AVG=$(grep "Level 2 writes" "$TEMP_OUTPUT" | grep -oE '[0-9]+\.[0-9]+ MB per compaction' | head -1 | grep -oE '[0-9]+\.[0-9]+' || echo "0")
    
    if [ "$MOD" = "10" ]; then
        echo "MOD=10 (Doux) 写入组成分析:"
        echo "  - Level 0: 主要是 flush 操作（memtable -> SST + VSST）"
        echo "  - Level 1+: compaction 操作（包括 SST 和 VSST 的 compaction）"
        echo ""
        if [ "$LEVEL0_WRITE" != "0" ]; then
            LEVEL0_MB=$(echo "scale=2; $LEVEL0_WRITE / 1024 / 1024" | bc)
            echo "  Level 0 写入: $LEVEL0_WRITE 字节 ($LEVEL0_MB MB)"
        fi
        if [ "$LEVEL1_WRITE" != "0" ]; then
            LEVEL1_MB=$(echo "scale=2; $LEVEL1_WRITE / 1024 / 1024" | bc)
            echo "  Level 1 写入: $LEVEL1_WRITE 字节 ($LEVEL1_MB MB)"
        fi
        if [ "$LEVEL2_WRITE" != "0" ]; then
            LEVEL2_MB=$(echo "scale=2; $LEVEL2_WRITE / 1024 / 1024" | bc)
            echo "  Level 2 写入: $LEVEL2_WRITE 字节 ($LEVEL2_MB MB)"
        fi
        echo ""
        echo "  注意: MOD=10 中大部分数据在 VSST 中，确保所有 VSST 写入都被统计"
    elif [ "$MOD" = "0" ]; then
        echo "MOD=0 (LevelDB) 写入组成分析:"
        echo "  - Level 0: flush 操作（memtable -> SST）"
        echo "  - Level 1+: compaction 操作（合并不同 level 的 SST 文件）"
        echo ""
        if [ "$LEVEL0_WRITE" != "0" ]; then
            LEVEL0_MB=$(echo "scale=2; $LEVEL0_WRITE / 1024 / 1024" | bc)
            echo "  Level 0 写入: $LEVEL0_WRITE 字节 ($LEVEL0_MB MB)"
        fi
        if [ "$LEVEL1_WRITE" != "0" ]; then
            LEVEL1_MB=$(echo "scale=2; $LEVEL1_WRITE / 1024 / 1024" | bc)
            echo "  Level 1 写入: $LEVEL1_WRITE 字节 ($LEVEL1_MB MB)"
        fi
        if [ "$LEVEL2_WRITE" != "0" ]; then
            LEVEL2_MB=$(echo "scale=2; $LEVEL2_WRITE / 1024 / 1024" | bc)
            echo "  Level 2 写入: $LEVEL2_WRITE 字节 ($LEVEL2_MB MB)"
        fi
    fi
    echo ""
fi

# 检查统计验证信息
if grep -q "Statistics verification" "$TEMP_OUTPUT"; then
    echo "统计验证:"
    grep "Statistics verification" "$TEMP_OUTPUT"
    echo ""
fi

echo "说明:"
echo "  - Level 0: 主要是 flush 操作（memtable -> SST，MOD=10 还包括 VSST）"
echo "  - Level 1+: compaction 操作（合并不同 level 的文件）"
echo "  - 如果只有 Level 0 有写入，说明只触发了 flush，没有 compaction"
echo "  - 如果 Level 1+ 也有写入，说明触发了 compaction，写放大会更高"
if [ "$MOD" = "10" ]; then
    echo "  - MOD=10 中，VSST compaction 的写入应该被统计到对应的 level"
fi
echo ""

# 显示 Compaction 统计摘要
if [ -n "$TOTAL_COMPACTION_COUNT" ] && [ "$TOTAL_COMPACTION_COUNT" -gt 0 ]; then
    echo "=========================================="
    echo "Compaction 统计摘要"
    echo "=========================================="
    echo "总 Compaction 次数: $TOTAL_COMPACTION_COUNT"
    echo "  - Level 0 (Flush): $LEVEL0_COUNT 次"
    if [ "$LEVEL1_COUNT" -gt 0 ]; then
        echo "  - Level 1 (Compaction): $LEVEL1_COUNT 次, 平均每次 ${LEVEL1_AVG} MB"
    fi
    if [ "$LEVEL2_COUNT" -gt 0 ]; then
        echo "  - Level 2 (Compaction): $LEVEL2_COUNT 次, 平均每次 ${LEVEL2_AVG} MB"
    fi
    echo ""
fi

rm -f "$TEMP_OUTPUT"
if [ -n "$STRACE_OUTPUT" ] && [ -f "$STRACE_OUTPUT" ]; then
    rm -f "$STRACE_OUTPUT"
fi

# 等待一下确保所有文件都写入完成
sleep 2

# 记录加载后的目录大小（用于对比）
AFTER_SIZE=$(du -sb "$DB_DIR" 2>/dev/null | cut -f1)
if [ -z "$AFTER_SIZE" ]; then
    echo "错误: 无法获取数据库目录大小"
    exit 1
fi
AFTER_SIZE_MB=$(echo "scale=2; $AFTER_SIZE / 1024 / 1024" | bc)
AFTER_SIZE_GB=$(echo "scale=2; $AFTER_SIZE / 1024 / 1024 / 1024" | bc)

echo "=========================================="
echo "写入统计信息"
echo "=========================================="
echo "加载后数据库目录大小（最终数据量）:"
echo "  $AFTER_SIZE 字节"
echo "  $AFTER_SIZE_MB MB"
echo "  $AFTER_SIZE_GB GB"
echo ""

if [ $TOTAL_BYTES_WRITTEN -gt 0 ]; then
    TOTAL_WRITE_MB=$(echo "scale=2; $TOTAL_BYTES_WRITTEN / 1024 / 1024" | bc)
    TOTAL_WRITE_GB=$(echo "scale=2; $TOTAL_BYTES_WRITTEN / 1024 / 1024 / 1024" | bc)
    echo "进程全程写入磁盘数据量（包括所有 flush 和 compaction）:"
    echo "  $TOTAL_BYTES_WRITTEN 字节"
    echo "  $TOTAL_WRITE_MB MB"
    echo "  $TOTAL_WRITE_GB GB"
    echo "  说明: 这包括所有写入操作，即使数据在后续 compaction 中被删除/覆盖"
    echo ""
    
    # 计算实际写入量（目录大小增量，用于对比）
    ACTUAL_WRITE_SIZE=$((AFTER_SIZE - BEFORE_SIZE))
    ACTUAL_WRITE_MB=$(echo "scale=2; $ACTUAL_WRITE_SIZE / 1024 / 1024" | bc)
    
    echo "最终数据库目录大小（本次加载后）:"
    echo "  $AFTER_SIZE 字节"
    echo "  $AFTER_SIZE_MB MB"
    echo "  说明: 这是最终持久化存储的数据，不包括在 compaction 中被删除的数据"
    echo ""
    
    # 计算被删除的数据量
    DELETED_SIZE=$((TOTAL_BYTES_WRITTEN - ACTUAL_WRITE_SIZE))
    if [ $DELETED_SIZE -gt 0 ]; then
        DELETED_MB=$(echo "scale=2; $DELETED_SIZE / 1024 / 1024" | bc)
        DELETED_PERCENT=$(echo "scale=2; $DELETED_SIZE * 100 / $TOTAL_BYTES_WRITTEN" | bc)
        echo "在 compaction 过程中被删除的数据量:"
        echo "  $DELETED_SIZE 字节"
        echo "  $DELETED_MB MB"
        echo "  占写入总量的: ${DELETED_PERCENT}%"
        echo ""
    fi
    
    # 使用总写入量计算写放大（这是正确的写放大）
    WRITE_AMPLIFICATION=$(echo "scale=4; $TOTAL_BYTES_WRITTEN / $LOGICAL_WRITE_SIZE" | bc)
    WRITE_AMPLIFICATION_PERCENT=$(echo "scale=2; ($TOTAL_BYTES_WRITTEN - $LOGICAL_WRITE_SIZE) * 100 / $LOGICAL_WRITE_SIZE" | bc)
    
    # 计算压缩比（最终大小 / 总写入量）
    if [ $TOTAL_BYTES_WRITTEN -gt 0 ]; then
        COMPRESSION_RATIO=$(echo "scale=4; $AFTER_SIZE / $TOTAL_BYTES_WRITTEN" | bc)
    else
        COMPRESSION_RATIO="N/A"
    fi
    
    echo "=========================================="
    echo "写放大分析结果"
    echo "=========================================="
    echo "逻辑写入量（用户数据）:"
    echo "  $LOGICAL_WRITE_SIZE 字节 ($LOGICAL_WRITE_MB MB)"
    echo ""
    echo "进程全程写入磁盘数据量（包括所有操作）:"
    echo "  $TOTAL_BYTES_WRITTEN 字节 ($TOTAL_WRITE_MB MB)"
    echo ""
    echo "最终数据库目录大小:"
    echo "  $AFTER_SIZE 字节 ($AFTER_SIZE_MB MB)"
    echo ""
    echo "写放大系数（总写入量 / 逻辑写入量）: $WRITE_AMPLIFICATION"
    echo "额外写入比例: ${WRITE_AMPLIFICATION_PERCENT}%"
    if [ "$COMPRESSION_RATIO" != "N/A" ]; then
        echo "压缩比（最终大小 / 总写入量）: $COMPRESSION_RATIO"
    fi
    echo "=========================================="
    echo ""
    
    # 写放大评估
    WA_VALUE=$(echo "scale=2; $WRITE_AMPLIFICATION" | bc)
    if (( $(echo "$WA_VALUE <= 1.1" | bc -l) )); then
        echo "✓ 写放大评估: 优秀 (<= 1.1)"
    elif (( $(echo "$WA_VALUE <= 1.5" | bc -l) )); then
        echo "✓ 写放大评估: 良好 (1.1-1.5)"
    elif (( $(echo "$WA_VALUE <= 2.0" | bc -l) )); then
        echo "⚠ 写放大评估: 一般 (1.5-2.0)"
    else
        echo "✗ 写放大评估: 较差 (> 2.0)"
    fi
    echo ""
    
    echo "说明:"
    echo "  - 写放大系数 = 进程全程写入量 / 逻辑写入量"
    echo "  - 进程全程写入量包括："
    echo "    1. 所有 flush 操作写入的数据（memtable -> SST）"
    echo "    2. 所有 compaction 操作写入的数据（包括后续被删除的数据）"
    echo "  - 最终目录大小只包含最终保留的数据，不包括在 compaction 中被删除的数据"
    echo "  - 写放大反映了数据在写入过程中被重复写入的次数"
    echo "  - 理想情况下，写放大应该接近 1.0（只写入一次）"
    echo "  - 额外写入主要来自 compaction 操作（数据在不同 level 之间移动时被重新写入）"
    echo ""
    echo "对比参考:"
    if [ "$MOD" = "0" ]; then
        echo "  - LevelDB (MOD=0) 典型写放大: 2.0 - 3.0"
        echo "  - 当前测试结果 (MOD=0): $WRITE_AMPLIFICATION"
        if (( $(echo "$WRITE_AMPLIFICATION < 1.5" | bc -l) )); then
            echo ""
            echo "⚠ 注意: LevelDB 的写放大看起来偏小，可能原因："
            echo "  1. 数据量还不够大，没有触发足够的 compaction"
            echo "  2. 统计可能不完整（WAL 日志写入未统计）"
            echo "  3. 建议使用更大的数据量测试（如 60000000 条）"
        fi
    elif [ "$MOD" = "10" ]; then
        echo "  - LevelDB (MOD=0) 典型写放大: 2.0 - 3.0"
        echo "  - Doux (MOD=10) 当前测试结果: $WRITE_AMPLIFICATION"
        echo "  - 如果 Doux 的写放大低于 LevelDB，说明优化有效"
    else
        echo "  - LevelDB 典型写放大: 2.0 - 3.0"
        echo "  - RocksDB 典型写放大: 1.5 - 2.5"
        echo "  - 您的方案 (MOD=$MOD): $WRITE_AMPLIFICATION"
    fi
    echo ""
    
    # 数据量分析
    LOGICAL_GB=$(echo "scale=2; $LOGICAL_WRITE_SIZE / 1073741824" | bc)
    if (( $(echo "$LOGICAL_GB < 5" | bc -l) )); then
        echo "⚠ 数据量提示:"
        echo "  - 当前数据量: $LOGICAL_GB GB"
        echo "  - 数据量较小，可能只触发了 flush，未触发足够的 compaction"
        echo "  - 建议使用更大的数据量测试（如 60000000 条，约 18.8 GB）"
        echo "  - 更大的数据量会触发更多 compaction，写放大可能会增加"
        echo ""
    fi
else
    echo "警告: 无法从程序输出中获取总写入量统计信息"
    echo "      将使用目录大小增量来计算写放大（可能不准确）"
    echo ""
    
    # 回退到使用目录大小增量
    ACTUAL_WRITE_SIZE=$((AFTER_SIZE - BEFORE_SIZE))
    ACTUAL_WRITE_MB=$(echo "scale=2; $ACTUAL_WRITE_SIZE / 1024 / 1024" | bc)
    ACTUAL_WRITE_GB=$(echo "scale=2; $ACTUAL_WRITE_SIZE / 1024 / 1024 / 1024" | bc)
    
    echo "目录大小增量（本次加载）:"
    echo "  $ACTUAL_WRITE_SIZE 字节"
    echo "  $ACTUAL_WRITE_MB MB"
    echo "  $ACTUAL_WRITE_GB GB"
    echo ""
    
    if [ $LOGICAL_WRITE_SIZE -gt 0 ]; then
        WRITE_AMPLIFICATION=$(echo "scale=4; $ACTUAL_WRITE_SIZE / $LOGICAL_WRITE_SIZE" | bc)
        WRITE_AMPLIFICATION_PERCENT=$(echo "scale=2; ($ACTUAL_WRITE_SIZE - $LOGICAL_WRITE_SIZE) * 100 / $LOGICAL_WRITE_SIZE" | bc)
        
        echo "=========================================="
        echo "写放大结果（基于目录大小增量）:"
        echo "=========================================="
        echo "逻辑写入量: $LOGICAL_WRITE_SIZE 字节 ($LOGICAL_WRITE_MB MB)"
        echo "目录大小增量: $ACTUAL_WRITE_SIZE 字节 ($ACTUAL_WRITE_MB MB)"
        echo "写放大系数: $WRITE_AMPLIFICATION"
        echo "额外写入比例: ${WRITE_AMPLIFICATION_PERCENT}%"
        echo "=========================================="
        echo ""
        echo "注意: 此结果可能不准确，因为最终目录大小可能小于实际写入量"
    else
        echo "错误: 无法计算写放大，逻辑写入量为0"
        exit 1
    fi
fi

echo ""
echo "测量完成！"
