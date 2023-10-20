#!/bin/bash
set -e

cd ./build/output
cp ../../src/client/python/TuGraphClient/TuGraphClient.py .
cp ../../src/client/python/TuGraphClient/TuGraphRestClient.py .
cp -r ../../test/integration/* ./
cp -r ../../learn/examples/* ./
cp -r ../../demo/movie .
pytest ./
