```
rm -rf /home/ysm/work/test/leveldb_db_10
./tpch_bench_load_data -m 0 -f "/home/ysm/work/tpch-data/lineitem.10" -c 60000000 -d "/home/ysm/work/test/leveldb_db_10"  
./tpch_bench_load_data -m 0 -f "/home/ysm/work/tpch-data/lineitem.1" -c 6000000 -d "/home/ysm/work/test/leveldb_db_1"  

./tpch_bench_query_q6 -m 0 -f "/home/ysm/work/NEXT/tpch_query/query_q6_001" -q 5 -d "/home/ysm/work/test/leveldb_db_10" 
```