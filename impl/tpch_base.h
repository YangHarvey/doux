#ifndef TPCH_BASE_H
#define TPCH_BASE_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <ctime>
#include "leveldb/slice.h"
#include "util/coding.h"

using std::string;
using std::vector;
using namespace leveldb;

/* TPCH lineitem
 * l_orderkey       uint64_t(8)    pk1
 * l_partkey        uint64_t(8)    
 * l_suppkey        uint64_t(8)    
 * l_linenumber     uint64_t(8)    pk2
 * l_quantity       uint32_t(4)    
 * l_extendedprice  double(8)
 * l_discount       double(8)
 * l_tax            double(8)
 * l_returnflag     char(1)
 * l_linestatus     char(1)
 * l_shipdate       uint32_t(4)
 * l_commitdate     uint32_t(4)
 * l_receiptdate    uint32_t(4)
 * l_shipinstruct   char(64)
 * l_shipmode       char(64)
 * l_comment        char(128)       
 */

enum Lineitem {
    l_orderkey = 0,
    l_partkey = 1,
    l_suppkey = 2,
    l_linenumber = 3,
    l_quantity = 4,
    l_extendedprice = 5,
    l_discount = 6,
    l_tax = 7,
    l_returnflag = 8,   
    l_linestatus = 9,   
    l_shipdate = 10,    
    l_commitdate = 11,   
    l_receiptdate = 12,
    l_shipinstruct = 13, 
    l_shipmode = 14,     
    l_comment = 15     
};

constexpr size_t shipdate_offset = 0;
constexpr size_t quantity_offset = 4;
constexpr size_t discount_offset = 36;
constexpr size_t receiptdate_offset = 8;

const string start_date = "1992-01-01";
const string end_date = "1998-12-31";

inline void splitRow(const string& src, vector<string>& res, string delim = "|") {
    size_t size = src.size();
    size_t pos = 0;
    for (size_t i = 0; i < size; ++i) {
        pos = src.find(delim, i);
        if (pos < size) {
            string s = src.substr(i, pos - i);
            res.push_back(s);
            i = pos + delim.size() - 1;
        }
    }
}

inline uint32_t gapDays(const string& date1, const string& date2) {
    std::tm tm1, tm2;
    std::memset(&tm1, 0, sizeof(tm1));
    std::memset(&tm2, 0, sizeof(tm2));
    strptime(date1.data(), "%Y-%m-%d", &tm1);
    strptime(date2.data(), "%Y-%m-%d", &tm2);
    uint32_t gapSecs = static_cast<uint32_t>(std::difftime(std::mktime(&tm2), std::mktime(&tm1)));
    return gapSecs / 86400;
}

inline uint32_t uniformDate(const string& cur_date) {
    uint32_t all_days = gapDays(start_date, end_date);
    uint32_t cur_days = gapDays(start_date, cur_date);
    return cur_days * 1000 / all_days;
} 


constexpr size_t sk_shipdate_offset = 8;
constexpr size_t sk_quantity_offset = sk_shipdate_offset + sizeof(uint32_t);
constexpr size_t sk_receiptdate_offset = sk_quantity_offset + sizeof(uint32_t);
constexpr size_t sk_orderkey_offset = sk_receiptdate_offset + sizeof(uint32_t);
constexpr size_t sk_linenumber_offset = sk_orderkey_offset + 16;

inline bool sk_compare(const Slice &sk1, const Slice &sk2) {
    uint32_t shipdate1 = DecodeBigEndianFixed32(sk1.data() + sk_shipdate_offset);
    uint32_t shipdate2 = DecodeBigEndianFixed32(sk2.data() + sk_shipdate_offset);
    if (shipdate1 != shipdate2) {
        return shipdate1 < shipdate2;
    }

    uint32_t quantity1 = DecodeBigEndianFixed32(sk1.data() + sk_quantity_offset);
    uint32_t quantity2 = DecodeBigEndianFixed32(sk2.data() + sk_quantity_offset);
    if (quantity1 != quantity2) {
        return quantity1 < quantity2;
    }

    uint32_t receiptdate_1 = DecodeBigEndianFixed32(sk1.data() + sk_receiptdate_offset);
    uint32_t receiptdate_2 = DecodeBigEndianFixed32(sk2.data() + sk_receiptdate_offset);
    if (receiptdate_1 != receiptdate_2) {
        return receiptdate_1 < receiptdate_2;
    }

    // Compare orderkey using memcmp for better performance and correctness
    int orderkey_cmp = memcmp(sk1.data() + sk_orderkey_offset, sk2.data() + sk_orderkey_offset, 16);
    if (orderkey_cmp != 0) {
        return orderkey_cmp < 0;
    }

    // Compare linenumber using memcmp (you may need to specify the length correctly)
    int linenumber_cmp = memcmp(sk1.data() + sk_linenumber_offset, sk2.data() + sk_linenumber_offset, 8);
    return linenumber_cmp <= 0;
}


#endif
