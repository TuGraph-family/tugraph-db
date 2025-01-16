#! /bin/bash

killall -9 kvstore
sleep 1

script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
cd $script_dir

rm -rf n1
rm -rf n2
rm -rf n3
rm -rf n4
