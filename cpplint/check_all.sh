#! /bin/bash
set -e
dir=$(dirname "$(readlink -f "$0")")
cd $dir/../
find src test include toolkits plugins enterprise -name "*.cpp" -o -name "*.h" | grep -v "/lmdb/" | xargs python3 ./cpplint/cpplint.py --quiet
