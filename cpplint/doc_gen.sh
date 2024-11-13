#! /bin/bash
set -e
yum install doxygen -y
python -m pip install --exists-action=w --no-cache-dir -r docs/requirements.txt
cd docs/zh-CN
#make html SPHINXOPTS=-W
make html
cd ../en-US
#make html SPHINXOPTS=-W
make html
