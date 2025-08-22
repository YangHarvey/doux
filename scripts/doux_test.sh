mkdir -p ../build
cd ../build
cmake ../../doux -DCMAKE_BUILD_TYPE=Release -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

lineitem1=/projects/tpch-data/lineitem.tbl.1
doux_dir=/projects/doux/tpch/doux_dir
doux_output=/projects/doux/tpch/doux_output

mkdir -p ${doux_dir}
mkdir -p ${doux_output}

./doux_test --init_db --run_query -f $lineitem1 -m 10 -i 5 -v 64 -d $doux_dir > $doux_output/tpch_q6_value64.txt