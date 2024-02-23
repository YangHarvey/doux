#!/bin/bash
cd ../eval_ycsb

echo '===== YCSB Insert ====='
echo 'LevelDB:'
cat ycsb_leveldb_insert.txt | grep 'Timer 10 MEAN'
echo 'WiscKey:'
cat ycsb_wisckey_insert.txt | grep 'Timer 10 MEAN'
echo 'DiffKV:'
cat ycsb_diffkv_insert.txt | grep 'Timer 10 MEAN'
echo 'Doux:'
cat ycsb_doux_insert.txt | grep 'Timer 10 MEAN'


echo '===== YCSB Update ====='
echo 'LevelDB:'
cat ycsb_leveldb_update.txt | grep 'Timer 10 MEAN'
echo 'WiscKey:'
cat ycsb_wisckey_update.txt | grep 'Timer 10 MEAN'
echo 'DiffKV:'
cat ycsb_diffkv_update.txt | grep 'Timer 10 MEAN'
echo 'Doux:'
cat ycsb_doux_update.txt | grep 'Timer 10 MEAN'


echo '===== YCSB Get ====='
echo 'LevelDB:'
cat ycsb_leveldb_get.txt | grep 'Timer 4 MEAN'
echo 'WiscKey:'
cat ycsb_wisckey_get.txt | grep 'Timer 4 MEAN'
echo 'DiffKV:'
cat ycsb_diffkv_get.txt | grep 'Timer 4 MEAN'
echo 'Doux:'
cat ycsb_doux_get.txt | grep 'Timer 4 MEAN'


echo '===== YCSB Scan ====='
echo 'LevelDB:'
cat ycsb_leveldb_scan.txt | grep 'Timer 13 MEAN'
echo 'WiscKey:'
cat ycsb_wisckey_scan.txt | grep 'Timer 13 MEAN'
echo 'DiffKV:'
cat ycsb_diffkv_scan.txt | grep 'Timer 13 MEAN'
echo 'Doux:'
cat ycsb_doux_scan.txt | grep 'Timer 13 MEAN'