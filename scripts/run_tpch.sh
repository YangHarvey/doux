cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

mkdir -p /home/lyu/data/tpch_leveldb
mkdir -p ../eval_tpch_1/


# LevelDB
./tpch_bench  