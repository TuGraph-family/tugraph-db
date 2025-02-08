#! /bin/bash
set -e
dir=$(dirname "$(readlink -f "$0")")
cd $dir/../
find src test include toolkits procedures -name "*.cpp" -o -name "*.h" | grep -v "/lmdb/" | grep -v "bolt_raft.pb" | grep -v "test/test_procedures/" | xargs python3 ./cpplint/cpplint.py --quiet
