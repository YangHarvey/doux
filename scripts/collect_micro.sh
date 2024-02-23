#!/bin/bash
cd ../eval_micro


echo '===== Random Load ====='
echo 'LevelDB:'
cat ycsb_leveldb_rl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_rl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_rl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_rl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_rl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_rl_64KB.txt | grep 'Timer 9 MEAN'

echo 'WiscKey:'
cat ycsb_wisckey_rl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_rl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_rl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_rl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_rl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_rl_64KB.txt | grep 'Timer 9 MEAN'

echo 'DiffKV:'
cat ycsb_diffkv_rl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_rl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_rl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_rl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_rl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_rl_64KB.txt | grep 'Timer 9 MEAN'

echo 'Doux:'
cat ycsb_doux_rl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_rl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_rl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_rl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_rl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_rl_64KB.txt | grep 'Timer 9 MEAN'




echo '===== Seq Load ====='
echo 'LevelDB:'
cat ycsb_leveldb_sl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_sl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_sl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_sl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_sl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_leveldb_sl_64KB.txt | grep 'Timer 9 MEAN'

echo 'WiscKey:'
cat ycsb_wisckey_sl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_sl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_sl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_sl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_sl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_wisckey_sl_64KB.txt | grep 'Timer 9 MEAN'

echo 'DiffKV:'
cat ycsb_diffkv_sl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_sl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_sl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_sl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_sl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_diffkv_sl_64KB.txt | grep 'Timer 9 MEAN'

echo 'Doux:'
cat ycsb_doux_sl_64B.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_sl_256B.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_sl_1KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_sl_4KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_sl_16KB.txt | grep 'Timer 9 MEAN'
cat ycsb_doux_sl_64KB.txt | grep 'Timer 9 MEAN'




echo '===== Random Read ====='
echo 'LevelDB:'
cat ycsb_leveldb_rr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_rr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_rr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_rr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_rr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_rr_64KB.txt | grep 'Timer 4 MEAN'

echo 'WiscKey:'
cat ycsb_wisckey_rr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_rr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_rr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_rr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_rr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_rr_64KB.txt | grep 'Timer 4 MEAN'

echo 'DiffKV:'
cat ycsb_diffkv_rr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_rr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_rr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_rr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_rr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_rr_64KB.txt | grep 'Timer 4 MEAN'

echo 'Doux:'
cat ycsb_doux_rr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_rr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_rr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_rr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_rr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_rr_64KB.txt | grep 'Timer 4 MEAN'




echo '===== Seq Read ====='
echo 'LevelDB:'
cat ycsb_leveldb_sr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_sr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_sr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_sr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_sr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_leveldb_sr_64KB.txt | grep 'Timer 4 MEAN'

echo 'WiscKey:'
cat ycsb_wisckey_sr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_sr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_sr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_sr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_sr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_wisckey_sr_64KB.txt | grep 'Timer 4 MEAN'

echo 'DiffKV:'
cat ycsb_diffkv_sr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_sr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_sr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_sr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_sr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_diffkv_sr_64KB.txt | grep 'Timer 4 MEAN'

echo 'Doux:'
cat ycsb_doux_sr_64B.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_sr_256B.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_sr_1KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_sr_4KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_sr_16KB.txt | grep 'Timer 4 MEAN'
cat ycsb_doux_sr_64KB.txt | grep 'Timer 4 MEAN'
