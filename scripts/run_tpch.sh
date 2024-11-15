cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10

# working directory
LevelDB_DIR=/projects/doux/tpch/leveldb_dir
LevelDB_SI_DIR=/projects/doux/tpch/leveldb_si_dir
Wisckey_DIR=/projects/doux/tpch/wisckey_dir
DiffKV_DIR=/projects/doux/tpch/diffkv_dir
RISE_DIR=/projects/doux/tpch/rise_dir

# output
LevelDB_OUTPUT=/projects/doux/tpch/leveldb_output
LevelDB_SI_OUTPUT=/projects/doux/tpch/leveldb_si_output
RISE_OUTPUT=/projects/doux/tpch/rise_output

mkdir -p ${LevelDB_DIR}
mkdir -p ${LevelDB_SI_DIR}
mkdir -p ${Wisckey_DIR}
mkdir -p ${DiffKV_DIR}

mkdir -p ${LevelDB_OUTPUT}
mkdir -p ${LevelDB_SI_OUTPUT}
mkdir -p ${RISE_OUTPUT}


# TPC-H Q6
# LevelDB
./tpch_bench --init_db -t 6 -f $lineitem1 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q6.txt
# LevelDB(SI)
./tpch_bench --init_db --si -t 6 -f $lineitem1 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_SI_OUTPUT/tpch_q6.txt
# RISE(DSI)
./tpch_bench --init_db --si -t 6 -f $lineitem1 -m 12 -i 5 -d $LevelDB_DIR > $RISE_OUTPUT/tpch_q6.txt

