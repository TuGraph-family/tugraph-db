#! /bin/bash
set -e
cd doc/zh-CN
make html SPHINXOPTS=-W
cd ../en-US
make html SPHINXOPTS=-W
