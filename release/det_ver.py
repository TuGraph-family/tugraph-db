#!/usr/bin/env python
import re

def get_ver():
    f = open('../core/defs.h','r')
    lines = f.readlines()
    for line in lines:
        if line.find('VER_MAJOR') > 0:
            ver_major = line.split("= ")[-1].split(";")[0]
        if line.find('VER_MINOR') > 0:
            ver_minor = line.split("= ")[-1].split(";")[0]
        if line.find('VER_PATCH') > 0:
            ver_patch = line.split("= ")[-1].split(";")[0]
    return ver_major + '.' + ver_minor + '.' + ver_patch

def replace_ver(file_name, pattern, curr_ver):
    f = open(file_name, 'r+')
    lines = f.readlines()
    f.seek(0,0)
    for line in lines:
        if re.match(pattern, line) is not None:
            print("updating %s" % file_name)
            #f.write(line.replace(old_ver, new_ver))
            new_line = re.sub(r'[0-9]+\.[0-9]+\.[0-9]+', curr_ver, line)
            f.write(new_line)
        else:
            f.readline()
    f.close()

curr_ver = get_ver()
print("current version: %s" % curr_ver)
replace_ver('distribution/rpm/lgraph.spec', 'Version: ', curr_ver)
replace_ver('distribution/deb/lgraph/DEBIAN/control', 'Version: ', curr_ver)
replace_ver('doc/LightningGraph-Tutorial.adoc', 'Version: ', curr_ver)
replace_ver('doc/LightningGraph-Rest-API.adoc', 'Version: ', curr_ver)
replace_ver('doc/LightningGraph-ImportTool.adoc', 'Version: ', curr_ver)
replace_ver('doc/embed_python/index.rst', 'Version: ', curr_ver)
replace_ver('doc/embed_cpp/Doxyfile', 'PROJECT_NUMBER         = ', curr_ver)

