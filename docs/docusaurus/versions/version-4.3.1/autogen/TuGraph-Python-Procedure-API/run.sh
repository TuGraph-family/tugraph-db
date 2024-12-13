# 0. install sphinx-build: yum install python3-sphinx
# 1. default lgrpah module path: ../../../build/output/lgraph_python.so
# 2. make lgraph_python
# 3. run this script
rm output -rf
mkdir -p _static _templates
sphinx-build -W -b html . output
