#!/usr/bin/env python
# This script should be processed under root
import re

def get_ver():
    f = open('Options.cmake','r')
    lines = f.readlines()
    for line in lines:
        if line.find('LGRAPH_VERSION_MAJOR') > 0:
            ver_major = line.split(" ")[-1].split(")")[0]
        if line.find('LGRAPH_VERSION_MINOR') > 0:
            ver_minor = line.split(" ")[-1].split(")")[0]
        if line.find('LGRAPH_VERSION_PATCH') > 0:
            ver_patch = line.split(" ")[-1].split(")")[0]
    return ver_major + '.' + ver_minor + '.' + ver_patch

def replace_ver(file_name, pattern, curr_ver):
    f = open(file_name, 'r+')
    lines = f.readlines()
    f.seek(0,0)
    for line in lines:
        if pattern in line:
            print("updating %s" % file_name)
            new_line = re.sub(r'[0-9]+\.[0-9]+\.[0-9]+', curr_ver, line)
            print(new_line)
            f.write(new_line)
        else:
            f.write(line)
    f.close()

curr_ver = get_ver()
print("current version: %s" % curr_ver)
replace_ver('docs/autogen/TuGraph-Python-Procedure-API/index.rst', 'Version: ', curr_ver)
replace_ver('docs/autogen/TuGraph-CPP-Procedure-API/Doxyfile', 'PROJECT_NUMBER         = ', curr_ver)
replace_ver('docs/en-US/source/5.developer-manual/6.interface/3.procedure/4.Python-procedure.rst', 'Version: ', curr_ver)
replace_ver('docs/en-US/source/5.developer-manual/6.interface/3.procedure/Doxyfile', 'PROJECT_NUMBER         = ', curr_ver)
replace_ver('docs/zh-CN/source/5.developer-manual/6.interface/3.procedure/4.Python-procedure.rst', 'Version: ', curr_ver)
replace_ver('docs/zh-CN/source/5.developer-manual/6.interface/3.procedure/Doxyfile', 'PROJECT_NUMBER         = ', curr_ver)
replace_ver('docs/zh-CN/source/1.guide.md', '安装', curr_ver)
replace_ver('docs/en-US/source/1.guide.md', 'runtime', curr_ver)

dockerfiles = [
    "tugraph-mini-runtime-centos7-Dockerfile",
    "tugraph-mini-runtime-centos8-Dockerfile",
    "tugraph-mini-runtime-ubuntu18.04-Dockerfile",
    "tugraph-runtime-centos7-Dockerfile",
    "tugraph-runtime-centos8-Dockerfile",
    "tugraph-runtime-ubuntu18.04-Dockerfile"
]
for file in dockerfiles:
    replace_ver('ci/images/' + file, 'COPY', curr_ver)
    replace_ver('ci/images/' + file, 'RUN dpkg', curr_ver)
    replace_ver('ci/images/' + file, 'RUN rpm', curr_ver)
