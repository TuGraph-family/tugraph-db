#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for d in $DIR/x64/Debug $DIR/x64/Release; do
    mkdir -p $d
    cp $DIR/../src/python/*.py $d
    cp -r $DIR/../restful/server/resource $d/
done
