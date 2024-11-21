#!/bin/bash
mkdir -p ../build
cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j


# working directory
LevelDB_DIR=/projects/doux/mircobenchmark/leveldb_dir
LevelDB_SI_DIR=/projects/doux/mircobenchmark/leveldb_si_dir
RISE_DIR=/projects/doux/mircobenchmark/rise_dir

# output
LevelDB_OUTPUT=/projects/doux/mircobenchmark/leveldb_output
LevelDB_SI_OUTPUT=/projects/doux/mircobenchmark/leveldb_si_output
RISE_OUTPUT=/projects/doux/mircobenchmark/rise_output

mkdir -p ${LevelDB_DIR}
mkdir -p ${LevelDB_SI_DIR}
mkdir -p ${RISE_DIR}

mkdir -p ${LevelDB_OUTPUT}
mkdir -p ${LevelDB_SI_OUTPUT}
mkdir -p ${RISE_OUTPUT}

# Random Load + Random Read + Seq Read

# Random Load

# LevelDB
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d ${LevelDB_DIR} -m 0 -t 3 > ${LevelDB_OUTPUT}/micro_leveldb_rl_64KB.txt


./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d ${RISE_DIR} -m 12 -t 3 > ${RISE_OUTPUT}/micro_rise_rl_64KB.txt


# Sequential Load
./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d ${LevelDB_DIR} -m 0 -t 0 > ${LevelDB_OUTPUT}/micro_leveldb_sl_64KB.txt


./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_64B.txt
./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_256B.txt
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_1KB.txt
./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_4KB.txt
./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_16KB.txt
./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d ${RISE_DIR} -m 12 -t 0 > ${RISE_OUTPUT}/micro_rise_sl_64KB.txt

# WiscKey
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_64B.txt

# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3885 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3885 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_256B.txt

# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_1KB.txt

# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_4KB.txt

# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_16KB.txt

# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rl_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_wisckey -m 8 -t 3 > ../eval/ycsb_wisckey_rr_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sr_64KB.txt



# # DiffKV
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_64B.txt

# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3855 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3855 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_256B.txt

# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_1KB.txt

# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_4KB.txt

# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_16KB.txt

# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rl_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_diffkv -m 9 -t 3 > ../eval/ycsb_diffkv_rr_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sr_64KB.txt



# # Doux
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_64B.txt
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_64B.txt
# ./ycsb_bench -k 16 -v 64 --unlimit_fd -w 2 -e 131072 -n 13107 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_64B.txt

# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3855 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_256B.txt
# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_256B.txt
# ./ycsb_bench -k 16 -v 256 --unlimit_fd -w 2 -e 38550 -n 3855 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_256B.txt

# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_1KB.txt
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 1008 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_1KB.txt

# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_4KB.txt
# ./ycsb_bench -k 16 -v 4096 --unlimit_fd -w 2 -e 2550 -n 255 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_4KB.txt

# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_16KB.txt
# ./ycsb_bench -k 16 -v 16384 --unlimit_fd -w 2 -e 639 -n 64 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_16KB.txt

# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rr_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_doux -m 10 -t 3 > ../eval/ycsb_doux_rl_64KB.txt
# ./ycsb_bench -k 16 -v 65536 --unlimit_fd -w 2 -e 159 -n 16 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sr_64KB.txt



# # Seq Load
# # LevelDB
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_64B.txt
# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_256B.txt
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_1KB.txt
# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_4KB.txt
# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_16KB.txt
# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_leveldb -m 0 -t 0 > ../eval/ycsb_leveldb_sl_64KB.txt


# # WiscKey
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_64B.txt
# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_256B.txt
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_1KB.txt
# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_4KB.txt
# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_16KB.txt
# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_wisckey -m 8 -t 0 > ../eval/ycsb_wisckey_sl_64KB.txt


# # DiffKV
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_64B.txt
# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_256B.txt
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_1KB.txt
# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_4KB.txt
# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_16KB.txt
# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_diffkv -m 9 -t 0 > ../eval/ycsb_diffkv_sl_64KB.txt


# # Doux
# ./ycsb_bench -k 16 -v 64 --init_db -e 131072 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_64B.txt
# ./ycsb_bench -k 16 -v 256 --init_db -e 38550 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_256B.txt
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_1KB.txt
# ./ycsb_bench -k 16 -v 4096 --init_db -e 2550 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_4KB.txt
# ./ycsb_bench -k 16 -v 16384 --init_db -e 639 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_16KB.txt
# ./ycsb_bench -k 16 -v 65536 --init_db -e 159 -d /home/lyu/data/ycsb_doux -m 10 -t 0 > ../eval/ycsb_doux_sl_64KB.txt
