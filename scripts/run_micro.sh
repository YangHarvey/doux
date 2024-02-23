#!/bin/bash
cd ../build
source /opt/rh/devtoolset-8/enable

rm -rf *
cmake ../../doux -DCMAKE_C_COMPILER=/opt/rh/devtoolset-8/root/bin/gcc -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-8/root/bin/g++ -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# Random Load + Random Read + Seq Read

# LevelDB
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_64B.txt
./ycsb_bench -k 16 -v 64 -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_64B.txt
./ycsb_bench -k 16 -v 64 -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_64B.txt

./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_256B.txt
./ycsb_bench -k 16 -v 256 -c -w 2 -e 38550 -n 3885 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_256B.txt
./ycsb_bench -k 16 -v 256 -c -w 2 -e 38550 -n 3885 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_256B.txt

./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_1KB.txt
./ycsb_bench -k 16 -v 1024 -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_1KB.txt
./ycsb_bench -k 16 -v 1024 -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_1KB.txt

./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_4KB.txt
./ycsb_bench -k 16 -v 4096 -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_4KB.txt
./ycsb_bench -k 16 -v 4096 -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_4KB.txt

./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_16KB.txt
./ycsb_bench -k 16 -v 16384 -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_16KB.txt
./ycsb_bench -k 16 -v 16384 -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_16KB.txt

./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rl_64KB.txt
./ycsb_bench -k 16 -v 65536 -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_leveldb -m 0 -t 3 > ../eval_micro/ycsb_leveldb_rr_64KB.txt
./ycsb_bench -k 16 -v 65536 -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sr_64KB.txt



# WiscKey
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_64B.txt

./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3885 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3885 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_256B.txt

./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_1KB.txt

./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_4KB.txt

./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_16KB.txt

./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rl_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_wisckey -m 8 -t 3 > ../eval_micro/ycsb_wisckey_rr_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sr_64KB.txt



# DiffKV
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_64B.txt

./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3855 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3855 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_256B.txt

./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_1KB.txt

./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_4KB.txt

./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_16KB.txt

./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rl_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_diffkv -m 9 -t 3 > ../eval_micro/ycsb_diffkv_rr_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sr_64KB.txt



# Doux
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_64B.txt
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_64B.txt
./ycsb_bench -k 16 -v 64 --unlimit_fd -c -w 2 -e 131072 -n 13107 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_64B.txt

./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3855 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_256B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_256B.txt
./ycsb_bench -k 16 -v 256 --unlimit_fd -c -w 2 -e 38550 -n 3855 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_256B.txt

./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_1KB.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_1KB.txt
./ycsb_bench -k 16 -v 1024 --unlimit_fd -c -w 2 -e 10082 -n 1008 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_1KB.txt

./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_4KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_4KB.txt
./ycsb_bench -k 16 -v 4096 --unlimit_fd -c -w 2 -e 2550 -n 255 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_4KB.txt

./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_16KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_16KB.txt
./ycsb_bench -k 16 -v 16384 --unlimit_fd -c -w 2 -e 639 -n 64 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_16KB.txt

./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rr_64KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_doux -m 10 -t 3 > ../eval_micro/ycsb_doux_rl_64KB.txt
./ycsb_bench -k 16 -v 65536 --unlimit_fd -c -w 2 -e 159 -n 16 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sr_64KB.txt



# Seq Load
# LevelDB
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_leveldb -m 0 -t 0 > ../eval_micro/ycsb_leveldb_sl_64KB.txt


# WiscKey
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_wisckey -m 8 -t 0 > ../eval_micro/ycsb_wisckey_sl_64KB.txt


# DiffKV
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_diffkv -m 9 -t 0 > ../eval_micro/ycsb_diffkv_sl_64KB.txt


# Doux
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /data/doux/ycsb_doux -m 10 -t 0 > ../eval_micro/ycsb_doux_sl_64KB.txt
