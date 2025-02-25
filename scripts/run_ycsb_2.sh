#!/bin/bash
cd ../build
# source /opt/rh/devtoolset-9/enable

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

leveldb_dir=/projects/doux/ycsb/leveldb_dir
wiskcey_dir=/projects/doux/ycsb/wisckey_dir
diffkv_dir=/projects/doux/ycsb/diffkv_dir
doux_dir=/projects/doux/ycsb/doux_dir
rise_dir=/projects/doux/ycsb/rise_dir

leveldb_output=/projects/doux/ycsb/leveldb_output
doux_output=/projects/doux/ycsb/doux_output
rise_output=/projects/doux/ycsb/rise_output

mkdir -p ${leveldb_dir}
mkdir -p ${leveldb_output}
mkdir -p ${doux_dir}
mkdir -p ${doux_output}
mkdir -p ${rise_dir}
mkdir -p ${rise_output}

# # Insert
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/insert_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/insert.txt

# # Update
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/update_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/update.txt

# # Get
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/get_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 2 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/get.txt

# # Scan
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/scan_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 3 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/scan.txt

# # YCSB-A
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_a_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 4 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_a.txt

# # YCSB-B
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_b_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 5 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_b.txt

# # YCSB-C
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_c_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 6 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_c.txt

# # YCSB-D
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_d_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 7 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_d.txt

# # YCSB-E
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_e_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 8 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_e.txt

# # YCSB-F
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_f_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 9 -e 10082 -n 10082 -d ${leveldb_dir} -m 0 -t 3 > ${leveldb_output}/ycsb_f.txt


# Doux
# Insert
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/insert_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/insert.txt

# Update
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/update_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/update.txt

# Get
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/get_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/get.txt

# Scan
./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan_init.txt
sleep 10
./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 3 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan.txt

# # YCSB-A
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_a_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 4 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_a.txt

# # YCSB-B
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_b_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 5 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_b.txt

# # YCSB-C
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_c_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 6 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_c.txt

# # YCSB-D
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_d_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 7 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_d.txt

# # YCSB-E
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_e_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 8 -e 10082 -n 1008 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_e.txt

# # YCSB-F
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_f_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 9 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/ycsb_f.txt


# Rise
# Insert
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/insert_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/insert.txt


# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_f_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 9 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_f.txt




# Insert
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/insert_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/insert.txt

# # Update
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/update_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/update.txt

# # Get
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/get_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 2 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/get.txt

# # Scan
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/scan_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 3 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/scan.txt

# # YCSB-A
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_a_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 4 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_a.txt

# # YCSB-B
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_b_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 5 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_b.txt

# # YCSB-C
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_c_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 6 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_c.txt

# # YCSB-D
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_d_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 7 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_d.txt

# # YCSB-E
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_e_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 8 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_e.txt

# # YCSB-F
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_f_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 9 -e 10082 -n 10082 -d ${rise_dir} -m 12 -t 3 > ${rise_output}/ycsb_f.txt


