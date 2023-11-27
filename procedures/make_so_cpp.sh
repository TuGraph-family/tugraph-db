#!/bin/bash

APP=$1
APP_PATH=algo_cpp
INCLUDE_DIR="../include"
LIBLGRAPH="../build/output/liblgraph.so"


if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    g++ -fno-gnu-unique -DLGRAPH_ENABLE_LGRAPH_LOG=1 -fPIC -g --std=c++17 -I$INCLUDE_DIR -rdynamic -O3 -fopenmp -o ${APP}.so  $APP_PATH/${APP}_procedure.cpp $APP_PATH/${APP}_core.cpp $LIBLGRAPH -shared
elif [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -stdlib=libc++ -DLGRAPH_ENABLE_LGRAPH_LOG=1 -fPIC -g --std=c++17 -I$INCLUDE_DIR -rdynamic -O3 -Xpreprocessor -fopenmp -lomp -o ${APP}.so $APP_PATH/${APP}_procedure.cpp $APP_PATH/${APP}_core.cpp $LIBLGRAPH -shared
fi

