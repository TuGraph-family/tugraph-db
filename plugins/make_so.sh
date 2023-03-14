#!/bin/bash

INCLUDE_DIR="../include"
LIBLGRAPH="../build/output/liblgraph.so"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -fno-gnu-unique -fPIC -g --std=c++14 -I$INCLUDE_DIR -rdynamic -O3 -fopenmp -o $1.so  cpp/$11_procedure.cpp cpp/$1_core.cpp $LIBLGRAPH -shared
elif [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -stdlib=libc++ -fPIC -g --std=c++14 -I$INCLUDE_DIR -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -o $1.so cpp/$1_procedure.cpp cpp/$1_core.cpp $LIBLGRAPH -shared
fi

