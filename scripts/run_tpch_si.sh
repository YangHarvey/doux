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

# First range: 0.001 to 0.010, step 0.001
# for i in $(seq 1 10); do
#     selectivity_shipdate=$(printf "%.3f" $(echo "scale=3; $i * 0.001" | bc))
#     output_file=$(printf "tpch_q6_sf1_%04d.txt" $(echo "$selectivity_shipdate * 1000" | bc | xargs printf "%.0f"))
#     echo "Running with selectivity_shipdate = $selectivity_shipdate > ${LevelDB_SI_OUTPUT}/$output_file"
    
#     # Get initial disk read bytes
#     initial_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
#     ./tpch_selectivity --init_db --si -t 6 --predefined_range --s_shipdate $selectivity_shipdate --s_quantity 0.5 -f $lineitem1 -m 0 -i 1 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/$output_file
    
#     # Get final disk read bytes
#     final_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
#     # Calculate and append disk read info to output file
#     # Use bc for floating point arithmetic to avoid shell arithmetic errors
#     disk_read_diff=$(echo "$final_read - $initial_read" | bc)
#     echo "Disk read bytes: $disk_read_diff" >> ${LevelDB_SI_OUTPUT}/$output_file
# done

# Second range: 0.01 to 0.10, step 0.005
for i in $(seq 1 100); do
    selectivity_shipdate=$(printf "%.3f" $(echo "scale=3; $i * 0.001" | bc))
    output_file=$(printf "tpch_q6_sf1_%04d.txt" $(echo "$selectivity_shipdate * 1000" | bc | xargs printf "%.0f"))
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

# # Third range: 0.1 to 0.7, step 0.1
# for i in $(seq 1 7); do
#     selectivity_shipdate=$(printf "%.1f" $(echo "scale=1; $i * 0.1" | bc))
#     output_file=$(printf "tpch_q6_sf1_%04d.txt" $(echo "$selectivity_shipdate * 1000" | bc | xargs printf "%.0f"))
#     echo "Running with selectivity_shipdate = $selectivity_shipdate > ${LevelDB_SI_OUTPUT}/$output_file"
    
#     # Get initial disk read bytes
#     initial_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
#     ./tpch_selectivity --init_db --si -t 6 --predefined_range --s_shipdate $selectivity_shipdate --s_quantity 0.5 -f $lineitem1 -m 0 -i 1 -d $LevelDB_SI_DIR > ${LevelDB_SI_OUTPUT}/$output_file
    
#     # Get final disk read bytes
#     final_read=$(iostat -d -k 1 1 | grep -v "^$" | tail -n 1 | awk '{print $5}')
    
#     # Calculate and append disk read info to output file
#     # Use bc for floating point arithmetic to avoid shell arithmetic errors
#     disk_read_diff=$(echo "$final_read - $initial_read" | bc)
#     echo "Disk read bytes: $disk_read_diff" >> ${LevelDB_SI_OUTPUT}/$output_file
# done

