#!/bin/bash
cd ../build
# source /opt/rh/devtoolset-9/enable

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j


mkdir -p /home/lyu/data/ycsb_leveldb
mkdir -p /home/lyu/data/ycsb_wisckey
mkdir -p /home/lyu/data/ycsb_diffkv
mkdir -p /home/lyu/data/ycsb_doux
mkdir -p ../eval_ycsb_1/


# LevelDB
# Insert
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_insert_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_insert.txt

# Update
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_update_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_update.txt

# Get
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_get_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 2 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_get.txt

# Scan
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_scan_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 3 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_scan.txt

# YCSB-A
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_a_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 4 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_a.txt

# YCSB-B
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_b_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 5 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_b.txt

# YCSB-C
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_c_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 6 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_c.txt

# YCSB-D
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_d_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 7 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_d.txt

# YCSB-E
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_e_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 8 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_e.txt

# YCSB-F
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_f_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 9 -e 10082 -n 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 3 > ../eval_ycsb_1/ycsb_leveldb_ycsb_f.txt




# WiscKey
# Insert
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_insert_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 0 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_insert.txt

# Update
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_update_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 1 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_update.txt

# Get
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_get_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 2 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_get.txt

# Scan
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_scan_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 3 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_scan.txt

# YCSB-A
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_a_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 4 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_a.txt

# YCSB-B
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_b_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 5 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_b.txt

# YCSB-C
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_c_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 6 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_c.txt

# YCSB-D
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_d_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 7 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_d.txt

# YCSB-E
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_e_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 8 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_e.txt

# YCSB-F
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_f_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -w 9 -e 10082 -n 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval_ycsb_1/ycsb_wisckey_ycsb_f.txt




# DiffKV
# Insert
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_insert_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_insert.txt

# Update
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_update_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_update.txt

# Get
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_get_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_get.txt

# Scan
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_scan_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 3 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_scan.txt

# YCSB-A
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_a_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 4 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_a.txt

# YCSB-B
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_b_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 5 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_b.txt

# YCSB-C
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_c_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 6 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_c.txt

# YCSB-D
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_d_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 7 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_d.txt

# YCSB-E
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_e_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 8 -e 10082 -n 1008 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_e.txt

# YCSB-F
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_f_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 9 -e 10082 -n 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval_ycsb_1/ycsb_diffkv_ycsb_f.txt




# Doux
# Insert
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_insert_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_insert.txt

# Update
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_update_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_update.txt

# Get
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_get_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_get.txt

# Scan
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_scan_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 3 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_scan.txt

# YCSB-A
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_a_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 4 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_a.txt

# YCSB-B
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_b_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 5 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_b.txt

# YCSB-C
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_c_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 6 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_c.txt

# YCSB-D
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_d_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 7 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_d.txt

# YCSB-E
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_e_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 8 -e 10082 -n 1008 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_e.txt

# YCSB-F
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_f_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 9 -e 10082 -n 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval_ycsb_1/ycsb_doux_ycsb_f.txt