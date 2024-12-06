## Doux

Instructions in detail if you want to setup by yourself:

We provide scripts to reproduce the following experiments:

- `scripts/run_tpch.sh` reproduces how our system performs under TPCH workloads
- `scripts/run_ycsb.sh`  reproduces how our system performs under YCSB workloads
- `scripts/run_microbenchmark.sh` reproduces how our system performs under micorbenchmark workloads

To run all of them at once, please type in `scripts/run_all_expr.sh`.

Instructions in more detail if you want to do single testing by yourself:

If you want to run any of the single tests by yourself, please use the application "read_cold".
This is also what we use during the experiments. Some of the important options is listed below.
For a full option list, please use --help option.

-m: running mode. We keep a LevelDB and WiscKey baseline mode inside the library as well for ease of testing.
    "-m 0" is LevelDB,
    "-m 7" is Bourbon,
    "-m 8" is WiscKey,
    "-m 9" is DiffKV,
    "-m 10" is Doux.
-u: unlimited file descriptor. Baseline Wisckey (and LevelDB) assumes the system allows only
    1024 opened file descriptors for a process at the same time, so it hardcode its file 
    descriptor cache size to this value. We find it affect performance much with limited file 
    descriptors so we unlimit file descriptor to at most 65536 with option "-u". The machine 
    for artifact review has been configured to have unlimited file descriptor and please use 
    this option for testing.
-w: Use this option to perform a load.
-l: "-l 0" (default) means ordered loads and "-l 3" means random loads.
-i: number of iterations. Usually we use "-i 5" and use the average latency.
-n: number of requests (in thousands) in one iteration.
-f: the path to the dataset. The dataset file should contain one key per line.
-d: the path to the database. If with "-w" mode, this directory will be created or cleaned if
    already exists.
--distribution: the path to the request distribution file. A distribution file should contain
    one number "x" per line which indicate the x-th key in the dataset file
--YCSB: the path to the YCSB trace file. A YCSB trace file should contain two number "m x" per
    line, where m=0 means get and m=1 means put and x indicate the x-th key in the dataset file.
-k, -v: the size of keys and values, in bytes.
-c: cold read. With this option, before the workload the memory of the machine is cleared so 
    that the database resides on the disk. Use this only if you have sudo access without 
    entering a password.
--change_level_load: This option will disable using existing level models. Using this option
    will test on Sazerac-File (which is what we report), otherwise Sazerac-Level.

To interpret the output results, please check the last few lines of the output.
- Timer 13 is the total time, which is the time we report.
- Timer 4 is the total time for all get requests.
- Timer 10 is the total time for all put requests.
- Timer 7 is the total compaction time.
- Timer 11 is the total file model learning time.
