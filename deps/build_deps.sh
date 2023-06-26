#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- $0 )" >/dev/null 2>&1 && pwd )"
echo "======================================"
echo "Building tugraph-web"
echo "======================================"
cd ${SCRIPT_DIR}/tugraph-web
npm i
npm run build
RESOURCE_DIR=${SCRIPT_DIR}/../src/restful/server/resource/
rm -rf ${RESOURCE_DIR}
cp -r dist ${RESOURCE_DIR}
cd -
