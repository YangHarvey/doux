#!/bin/bash
cd ../build
# source /opt/rh/devtoolset-9/enable

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

doux_dir=/projects/doux/ycsb/doux_dir
doux_0_dir=/projects/doux/ycsb/doux_0_dir
doux_a_dir=/projects/doux/ycsb/doux_a_dir
doux_b_dir=/projects/doux/ycsb/doux_b_dir

doux_a_output=/projects/doux/ycsb/doux_a_output
doux_b_output=/projects/doux/ycsb/doux_b_output
doux_output=/projects/doux/ycsb/doux_output
doux_0_output=/projects/doux/ycsb/doux_0_output

mkdir -p ${doux_dir}
mkdir -p ${doux_output}
mkdir -p ${doux_a_output}
mkdir -p ${doux_b_output}
mkdir -p ${doux_0_output}
mkdir -p ${doux_0_dir}

# Doux-0
# Insert
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=false -e 100862 -d ${doux_0_dir} -m 10 -t 3 > ${doux_0_output}/insert_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 0 --use_dropmap=false --decoupled_compaction=false -e 100862 -n 10082 -d ${doux_0_dir} -m 10 -t 3 > ${doux_0_output}/insert.txt
# # Update
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/update_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 1 --use_dropmap=false --decoupled_compaction=false -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/update.txt
# # Get
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/get_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 2 --use_dropmap=false --decoupled_compaction=false -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/get.txt
# # Scan
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/scan_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 3 --use_dropmap=false --decoupled_compaction=false -e 10082 -n 10 -d ${doux_dir} -m 10 -t 3 > ${doux_0_output}/scan.txt


# # Doux-A
# # Insert
./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=true -e 100862 -d ${doux_a_dir}_insert -m 10 -t 3 > ${doux_a_output}/insert_init.txt
sleep 10
./ycsb_bench_2 -k 16 -v 1024 -c -w 0 --use_dropmap=false --decoupled_compaction=true -e 100862 -n 100862 -d ${doux_a_dir}_insert -m 10 -t 3 > ${doux_a_output}/insert.txt
# Update
./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=true -e 100862 -d ${doux_a_dir}_update -m 10 -t 3 > ${doux_a_output}/update_init.txt
sleep 10
./ycsb_bench_2 -k 16 -v 1024 -c -w 1 --use_dropmap=false --decoupled_compaction=true -e 100862 -n 100862 -d ${doux_a_dir}_update -m 10 -t 3 > ${doux_a_output}/update.txt
# # Get
./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=true -e 100862 -d ${doux_a_dir}_get -m 10 -t 3 > ${doux_a_output}/get_init.txt
sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 2 --use_dropmap=false --decoupled_compaction=true -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_a_output}/get.txt
# # Scan
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=false --decoupled_compaction=true -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_a_output}/scan_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 3 --use_dropmap=false --decoupled_compaction=true -e 10082 -n 10 -d ${doux_dir} -m 10 -t 3 > ${doux_a_output}/scan.txt


# # Doux-B
# # Insert
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=true --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/insert_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 0 --use_dropmap=true --decoupled_compaction=false -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/insert.txt
# # Update
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=true --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/update_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 1 --use_dropmap=true --decoupled_compaction=false -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/update.txt
# # Get
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=true --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/get_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 2 --use_dropmap=true --decoupled_compaction=false -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/get.txt
# # Scan
# ./ycsb_bench_2 -k 16 -v 1024 --init_db --use_dropmap=true --decoupled_compaction=false -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/scan_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 3 --use_dropmap=true --decoupled_compaction=false -e 10082 -n 10 -d ${doux_dir} -m 10 -t 3 > ${doux_b_output}/scan.txt

# Doux
# Insert
# ./ycsb_bench_2 -k 16 -v 1024 --init_db -e 100862 -d ${doux_dir}_insert -m 10 -t 3 > ${doux_output}/insert_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 0 -e 100862 -n 10082 -d ${doux_dir}_insert -m 10 -t 3 > ${doux_output}/insert.txt
# # Update
# ./ycsb_bench_2 -k 16 -v 1024 --init_db -e 100862 -d ${doux_dir}_update -m 10 -t 3 > ${doux_output}/update_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 1 -e 100862 -n 100862 -d ${doux_dir}_update -m 10 -t 3 > ${doux_output}/update.txt
# Get
# ./ycsb_bench_2 -k 16 -v 1024 --init_db -e 100862 -d ${doux_dir}_get -m 10 -t 3 > ${doux_output}/get_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 2 -e 100862 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/get.txt
# # Scan
# ./ycsb_bench_2 -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan_init.txt
# sleep 10
# ./ycsb_bench_2 -k 16 -v 1024 -c -w 3 -e 10082 -n 10 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan.txt




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
# # Insert
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/insert_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 0 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/insert.txt

# # Update
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/update_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 -c -w 1 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/update.txt

# # Get
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/get_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 2 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/get.txt

# # Scan
# ./ycsb_bench -k 16 -v 1024 --init_db -e 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan_init.txt
# sleep 10
# ./ycsb_bench -k 16 -v 1024 --unlimit_fd -w 3 -e 10082 -n 10082 -d ${doux_dir} -m 10 -t 3 > ${doux_output}/scan.txt

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


