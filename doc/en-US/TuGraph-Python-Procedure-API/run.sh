# 1. default lgrpah module path: ../../../cmake-build-debug/output/lgraph_python.so
# 2. make lgraph module using python2
# edit src/BuildPythonPackage.cmake: find_package(PythonLibs 2)
# then make lgraph_python
# 3. run this script
rm output -rf
mkdir -p _static _templates
sphinx-build -b html . output
