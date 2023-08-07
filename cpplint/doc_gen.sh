#! /bin/bash
set -e
cd docs/zh-CN
make html SPHINXOPTS=-W
cd ../en-US
make html SPHINXOPTS=-W
