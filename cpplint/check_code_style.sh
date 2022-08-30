#! /bin/bash
set -e
dir=$(dirname "$(readlink -f "$0")")
cd $dir/../
files=`git diff --name-only HEAD HEAD~1`
echo "$files" | while read file
do
    if [[ $file =~ \.h$ || $file =~ \.cpp$ ]]; then
        if [[ -f $file ]]; then
            python3 ./cpplint/cpplint.py --quiet $file
            if [[ $? -ne 0 ]]; then
                exit 1
            fi
        fi
    fi
done
