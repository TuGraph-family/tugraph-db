#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- $0 )" >/dev/null 2>&1 && pwd )"
echo "======================================"
echo "Building tugraph-web"
echo "======================================"
cd ${SCRIPT_DIR}/tugraph-web
npm i
npm run lint -- --fix
npm run build
RESOURCE_DIR=${SCRIPT_DIR}/../src/restful/server/resource/
rm -rf ${RESOURCE_DIR}
rm -rf dist/scene_data
cp -r dist ${RESOURCE_DIR}
cd ..

echo "======================================"
echo "Building tugraph-db-browser"
echo "======================================"

cd tugraph-db-browser
yarn bootstrap
rm -rf client/dist
yarn build
BROWSER_RESOURCE_DIR=${SCRIPT_DIR}/../src/restful/server/browser-resource
rm -rf ${BROWSER_RESOURCE_DIR}
cp -r client/dist/resource ${BROWSER_RESOURCE_DIR}
cp -r client/src/constants/demo_data ${BROWSER_RESOURCE_DIR}/
