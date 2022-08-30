#!/usr/bin/python

import subprocess

p = subprocess.Popen(['dpkg-shlibdeps', '-O', '../lgraph/usr/local/lib64/liblgraph.so', '../lgraph/usr/local/bin/lgraph_server'], stdout=subprocess.PIPE, cwd='.')
p.wait()
substvars = p.stdout.read()
fin = open('debian/control', 'r')
fout = open('../lgraph/DEBIAN/control', 'w')
while True:
    line = fin.readline()
    if line == '': break
    if line.startswith('Depends:'):
        fout.write('Depends: %s' % substvars[len('shlibs:Depends='):])
    else:
        fout.write(line)
fin.close()
fout.close()
