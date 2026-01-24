# 启动程序并在后台运行
./tpch_bench_load_data -m 10 -f "/home/ysm/work/tpch-data/lineitem.100" -c 600000000 -d "/home/ysm/work/test/doux_db_100" & 
APP_PID=$!

# 在程序退出前截取峰值
echo "Peak RSS for this run:"
grep VmHWM /proc/$APP_PID/status