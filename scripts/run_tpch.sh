cd ../build

cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10
lineitem100=/projects/tpch-data/lineitem.tbl.100

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


# LevelDB Load Phase
./tpch_bench --init_db -f $lineitem10 -d $LevelDB_DIR --run_query=false > $LevelDB_OUTPUT/tpch_load_data.txt
# LevelDB Query Phase
./tpch_bench --run_query=true -t 6 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q6.txt
./tpch_bench --run_query=true -t 12 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q12.txt
./tpch_bench --run_query=true -t 14 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q14.txt
./tpch_bench --run_query=true -t 19 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q19.txt
./tpch_bench --run_query=true -t 20 -m 0 -i 5 -d $LevelDB_DIR > $LevelDB_OUTPUT/tpch_q20.txt

# LevelDB(SI) Load Phase
./tpch_bench --init_db -f $lineitem10 -d $LevelDB_SI_DIR --run_query=false > $LevelDB_SI_OUTPUT/tpch_load_data.txt
# LevelDB(SI) Query Phase
./tpch_bench --run_query=true -t 6  --si -m 0 -i 5 -d $LevelDB_SI_DIR > $LevelDB_SI_OUTPUT/tpch_q6.txt
./tpch_bench --run_query=true -t 12  --si -m 0 -i 5 -d $LevelDB_SI_DIR > $LevelDB_SI_OUTPUT/tpch_q12.txt
./tpch_bench --run_query=true -t 14  --si -m 0 -i 5 -d $LevelDB_SI_DIR > $LevelDB_SI_OUTPUT/tpch_q14.txt
./tpch_bench --run_query=true -t 19  --si -m 0 -i 5 -d $LevelDB_SI_DIR > $LevelDB_SI_OUTPUT/tpch_q19.txt
./tpch_bench --run_query=true -t 20  --si -m 0 -i 5 -d $LevelDB_SI_DIR > $LevelDB_SI_OUTPUT/tpch_q20.txt
