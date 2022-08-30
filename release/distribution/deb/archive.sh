#!/bin/bash
LOCAL_DIR=../../local
LGRAPH_DIR=lgraph
VERSION=`grep "Version:" lgraph/DEBIAN/control | awk '{print $2}'`

sudo rm ${LGRAPH_DIR}/usr -rf
sudo mkdir -p ${LGRAPH_DIR}/usr
sudo cp -r ${LOCAL_DIR} ${LGRAPH_DIR}/usr
sudo find ${LGRAPH_DIR}/usr -name ".gitkeep" -type f -delete

cd shlibdeps && sudo python shlibdeps.py && cd -

sudo chown -R root:root ${LGRAPH_DIR}
sudo dpkg -b lgraph/ lgraph_${VERSION}-0ubuntu1_amd64.deb
