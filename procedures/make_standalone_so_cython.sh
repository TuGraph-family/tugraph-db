#!/bin/bash

APP=$1
INCLUDE_PATH=../build/output
CYTHON_APP_PATH=./algo_cython

cython -+ -3 -I../src/cython -I../include/cython $CYTHON_APP_PATH/${APP}_standalone.py -o ${APP}_standalone.cpp
gcc -pthread -Wno-unused-result -Wsign-compare -DNDEBUG -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -fPIC -I../src -I../include -I/usr/include/python3.6m -I/usr/local/include/python3.6m -c ${APP}_standalone.cpp -o ${APP}_standalone.o -Wall -g -fno-gnu-unique -fPIC --std=c++17 -rdynamic -O3 -fopenmp
g++ -pthread -shared -Wl,-z,relro -g ${APP}_standalone.o -L$INCLUDE_PATH -L/usr/lib64 -llgraph -o ${APP}_standalone.so -Wall -g -fno-gnu-unique -fPIC --std=c++17 -rdynamic -O3 -fopenmp
rm ${APP}_standalone.o
rm ${APP}_standalone.cpp