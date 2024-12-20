#!/bin/bash

APP=$1
APP_PATH=algo_cpp
INCLUDE_DIR="../include"
LIBLGRAPH="../build/output/liblgraph.so"


if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -fno-gnu-unique -fPIC -g --std=c++17 -I$INCLUDE_DIR -rdynamic -O3 -fopenmp -static-libstdc++ -static-libgcc -o ${APP}.so $APP_PATH/${APP}_procedure.cpp $APP_PATH/${APP}_core.cpp $LIBLGRAPH -shared
elif [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I$INCLUDE_DIR -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -o ${APP}.so $APP_PATH/${APP}_procedure.cpp $APP_PATH/${APP}_core.cpp $LIBLGRAPH -shared
fi
