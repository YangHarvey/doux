cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10

# working directory
LevelDB_DIR=/projects/doux/tpch_leveldb
LevelDB_SI_DIR=/projects/doux/tpch_leveldb_si
Wisckey_DIR=/projects/doux/tpch_wisckey
DiffKV_DIR=/projects/doux/tpch_diffkv

# output
LevelDB_OUTPUT=../eval_tpch_1/leveldb_output.txt
LevelDB_SI_OUTPUT=../eval_tpch_1/leveldb_si_output.txt

mkdir -p ${LevelDB_DIR}
mkdir -p ${LevelDB_SI_DIR}
mkdir -p ${Wisckey_DIR}
mkdir -p ${DiffKV_DIR}
mkdir -p ../eval_tpch_1/


# LevelDB
# ./tpch_bench --init_db -t 6 -f $lineitem10 -m 0 -i 10 -d $LevelDB_DIR > $LevelDB_OUTPUT

# LevelDB(SI)
./tpch_bench --init_db --si -t 6 -f $lineitem1 -m 1 -i 10 -d $LevelDB_DIR > $LevelDB_SI_OUTPUT

# Wisckey
# ./tpch_bench --init_db -t 6 -f $lineitem1 -m 8 -i 10 -d $Wisckey_DIR

# Diffkv
# ./tpch_bench --init_db -t 6 -f $lineitem1 -m 9 -i 10 -d $DiffKV_DIR