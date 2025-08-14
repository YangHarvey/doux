mkdir -p ../build
cd ../build
cmake ../../doux -DCMAKE_BUILD_TYPE=Release -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10
lineitem100=/projects/tpch-data/lineitem.tbl.100

leveldb_dir=/projects/doux/tpch/leveldb_dir
leveldb_si_dir=/projects/doux/tpch/leveldb_si_dir
wisckey_dir=/projects/doux/tpch/wisckey_dir
diffkv_dir=/projects/doux/tpch/diffkv_dir
doux_dir=/projects/doux/tpch/doux_dir
rise_dir=/projects/doux/tpch/rise_dir

leveldb_output=/projects/doux/tpch/leveldb_output
leveldb_si_output=/projects/doux/tpch/leveldb_si_output
wisckey_output=/projects/doux/tpch/wisckey_output
diffkv_output=/projects/doux/tpch/diffkv_output
doux_output=/projects/doux/tpch/doux_output
rise_output=/projects/doux/tpch/rise_output

mkdir -p ${leveldb_dir}
mkdir -p ${leveldb_si_dir}
mkdir -p ${wisckey_dir}
mkdir -p ${diffkv_dir}
mkdir -p ${doux_dir}
mkdir -p ${rise_dir}

mkdir -p ${leveldb_output}
mkdir -p ${leveldb_si_output}
mkdir -p ${wisckey_output}
mkdir -p ${diffkv_output}
mkdir -p ${doux_output}
mkdir -p ${rise_output}

# leveldb
# ./tpch_bench_vary_value_size --init_db --run_query -t 6 -f $lineitem10 -m 0 -i 5 -v 314 -d ${leveldb_dir} > ${leveldb_output}/tpch_q6_value314.txt

# leveldb(SI)
./tpch_bench_vary_value_size --init_db --si --run_query -t 6 -f $lineitem10 -m 0 -i 5 -v 314 -d $leveldb_si_dir > $leveldb_si_output/tpch_q6_value314.txt

# wisckey
# ./tpch_bench_vary_value_size --init_db --run_query -t 6 -f $lineitem10 -m 8 -i 5 -v 314 -d $wisckey_dir > $wisckey_output/tpch_q6_value314.txt

# diffkv
# ./tpch_bench_vary_value_size --init_db --run_query -t 6 -f $lineitem10 -m 9 -i 5 -v 314 -d $diffkv_dir > $diffkv_output/tpch_q6_value314.txt

# doux 
./tpch_bench_vary_value_size --init_db --run_query -t 6 -f $lineitem10 -m 10 -i 5 -v 314 -d $doux_dir > $doux_output/tpch_q6_value314.txt

# rise(si)
# ./tpch_bench_vary_value_size --init_db --si --run_query -t 6 -f $lineitem10 -m 12 -i 5 -v 314 -d $rise_dir > $rise_output/tpch_q6_value314.txt

