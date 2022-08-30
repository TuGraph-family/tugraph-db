#!/bin/bash
LOCAL_DIR=../../local
VERSION=`grep "Version:" lgraph.spec | awk '{print $2}'`
LGRAPH_DIR=lgraph-${VERSION}

rm ${LGRAPH_DIR} -rf
mkdir -p ${LGRAPH_DIR}/usr
cp -r ${LOCAL_DIR} ${LGRAPH_DIR}/usr
find ${LGRAPH_DIR}/usr -name ".gitkeep" -type f -delete
cp ../../LICENSE ${LGRAPH_DIR}
cp configure ${LGRAPH_DIR}
tar czf ${LGRAPH_DIR}.tar.gz ${LGRAPH_DIR}
