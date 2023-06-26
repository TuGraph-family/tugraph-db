#!/bin/bash

echo "======================================"
echo "Building tugraph-web"
echo "======================================"
cd ${DEPS_DIR}/tugraph-web
npm i
npm run build
RESOURCE_DIR=${DEPS_DIR}/../src/restful/server/resource/
rm -rf ${RESOURCE_DIR}
cp -r dist ${RESOURCE_DIR}
cd -
