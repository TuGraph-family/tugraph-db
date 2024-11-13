#! /bin/bash
set -e
cd docs/zh-CN
#make html SPHINXOPTS=-W
make html
cd ../en-US
#make html SPHINXOPTS=-W
make html
