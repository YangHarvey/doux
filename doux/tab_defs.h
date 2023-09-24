#pragma once

#include <string>
#include <vector>

// 数据类型
enum ColType {
    INT, DOUBLE, DATE, CHAR
};
// 索引类型
enum IndexType {
    KEY, ZorderKey, NONE
};

// column meta
class ColMeta {
public:
    ColMeta() = delete;
    ColMeta(ColType col_type, IndexType index_type, std::string col_name, int col_len) : 
        col_type_(col_type), index_type_(index_type), col_name_(std::move(col_name)), col_len_(col_len){ }
public:
    ColType col_type_;
    IndexType index_type_;
    std::string col_name_;
    int col_len_;   // 字节数
};

// table meta
class TableMeta {
public:
    TableMeta() = delete;
    TableMeta(std::string tab_name, std::vector<ColMeta> cols) : tab_name_(tab_name), cols_(std::move(cols)){
        // 初始化record length
        record_length_ = 0;
        for(auto &col : cols) {
            record_length_ += col.col_len_;
        }
        // zorder length未初始化
        zorder_length_ = -1;
    }

public:
    const std::string tab_name_;
    const std::vector<ColMeta> cols_;
    int record_length_; // 原数据长度
    int zorder_length_; // 经过z order编码之后的length

};