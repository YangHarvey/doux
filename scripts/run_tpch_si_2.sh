cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10
lineitem100=/projects/tpch-data/lineitem.tbl.100

# working directory
LevelDB_DIR=/projects/doux/tpch/leveldb_dir
LevelDB_SI_DIR=/projects/doux/tpch/leveldb_si_dir
Doux_DIR=/projects/doux/tpch/doux_dir
RISE_SI_DIR=/projects/doux/tpch/rise_si_dir

# output
LevelDB_OUTPUT=/projects/doux/tpch/leveldb_output
LevelDB_SI_OUTPUT=/projects/doux/tpch/leveldb_si_output
Doux_OUTPUT=/projects/doux/tpch/doux_output
RISE_SI_OUTPUT=/projects/doux/tpch/rise_si_output

mkdir -p ${LevelDB_DIR}
mkdir -p ${LevelDB_OUTPUT}
mkdir -p ${LevelDB_SI_DIR}
mkdir -p ${LevelDB_SI_OUTPUT}
mkdir -p ${Doux_DIR}
mkdir -p ${Doux_OUTPUT}
mkdir -p ${RISE_SI_DIR}
mkdir -p ${RISE_SI_OUTPUT}

# TPC-H Q6

# leveldb
# ./tpch_bench_2 --init_db -t 6 -f $lineitem1 -m 0 -d $LevelDB_DIR > ${LevelDB_OUTPUT}/tpch_q6_sf1.txt
# ./tpch_bench_2 --init_db -t 6 -f $lineitem10 -m 0 -d $LevelDB_DIR > ${LevelDB_OUTPUT}/tpch_q6_sf10.txt
# ./tpch_bench_2 --init_db -t 6 -f $lineitem100 -m 0 -d $LevelDB_DIR > ${LevelDB_OUTPUT}/tpch_q6_sf100.txt


# leveldb si
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem1 -m 0 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6_sf1.txt
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem10 -m 0 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6_sf10.txt
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem100 -m 0 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6_sf100.txt

# doux
# ./tpch_bench_2 --init_db -t 6 -f $lineitem1 -m 10 -d $Doux_DIR > ${Doux_OUTPUT}/tpch_q6_sf1.txt
# ./tpch_bench_2 --init_db -t 6 -f $lineitem10 -m 10 -d $Doux_DIR > ${Doux_OUTPUT}/tpch_q6_sf10.txt
# ./tpch_bench_2 --init_db -t 6 -f $lineitem100 -m 0 -d $Doux_DIR > ${Doux_OUTPUT}/tpch_q6_sf100.txt


# rise
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem1 -m 0 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6_sf1.txt
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem10 -m 12 -d $RISE_SI_DIR > ${RISE_SI_OUTPUT}/tpch_q6_sf10.txt
# ./tpch_bench_2 --init_db --si -t 6 -f $lineitem100 -m 0 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6_sf100.txt

