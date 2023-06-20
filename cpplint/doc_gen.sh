#! /bin/bash
set -e
cd doc/zh-CN
make html
cd ../en-US
make html
