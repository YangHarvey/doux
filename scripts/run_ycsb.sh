#!/bin/bash
cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# WiscKey
# Insert
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 0 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_insert.txt

# Update
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 1 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_update.txt

# Get
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 2 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_get.txt

# Scan
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 3 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_scan.txt

# YCSB-A
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 4 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_a.txt

# YCSB-B
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 5 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_b.txt

# YCSB-C
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 6 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_c.txt

# YCSB-D
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 7 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_d.txt

# YCSB-E
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 8 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_e.txt

# YCSB-F
./ycsb_bench -k 16 -v 64 --init_db -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_init.txt
./ycsb_bench -k 16 -v 64 -w 9 -d /mnt/doux/ycsb_wisckey -m 8 -t 3 > ../evaluation/ycsb_wisckey_ycsb_f.txt
