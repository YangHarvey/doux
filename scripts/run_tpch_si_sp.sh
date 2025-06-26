cd ../build

rm -rf *
cmake ../../doux -DCMAKE_BUILD_TYPE=RELEASE -DNDEBUG_SWITCH=ON -DLEVEL_SWITCH=ON -DINTERNAL_TIMER_SWITCH=ON
make -j

# data path
lineitem1=/projects/tpch-data/lineitem.tbl.1
lineitem10=/projects/tpch-data/lineitem.tbl.10
lineitem100=/projects/tpch-data/lineitem.tbl.100

# working directory
LevelDB_SI_DIR=/projects/doux/tpch/leveldb_si_dir
# output
LevelDB_SI_OUTPUT=/projects/doux/tpch/leveldb_si_output

mkdir -p ${LevelDB_SI_DIR}
mkdir -p ${LevelDB_SI_OUTPUT}

# TPC-H Q6
# LevelDB(SI) 使用预定义范围
# 
# ./tpch_selectivity --init_db --si -t 6 -f $lineitem1 -m 0 -i 5 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/tpch_q6.txt

selectivity_candidate=(0.0126 0.0127)
for i in "${selectivity_candidate[@]}"; do
    selectivity_shipdate=$(printf "%.4f" $(echo "scale=4; $i" | bc))
    output_file=$(printf "tpch_q6_sf1_%05d.txt" $(echo "$selectivity_shipdate * 10000" | bc | xargs printf "%.0f"))
    echo "Running with selectivity_shipdate = $selectivity_shipdate > ${LevelDB_SI_OUTPUT}/$output_file"
    
    # Get initial disk read bytes
    initial_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
    ./tpch_selectivity --init_db --si -t 6 --predefined_range --s_shipdate $selectivity_shipdate --s_quantity 0.5 -f $lineitem1 -m 0 -i 1 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/$output_file
    
    # Get final disk read bytes
    final_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
    # Calculate and append disk read info to output file
    # Use bc for floating point arithmetic to avoid shell arithmetic errors
    disk_read_diff=$(echo "$final_read - $initial_read" | bc)
    echo "Disk read bytes: $disk_read_diff" >> ${LevelDB_SI_OUTPUT}/$output_file
done

