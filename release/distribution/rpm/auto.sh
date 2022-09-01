#!/bin/bash
RPM_BUILD=$HOME/rpmbuild

./build.centos7.3 -j4
./archive.sh

\cp -f lgraph-*.*.*.tar.gz $RPM_BUILD/SOURCES
\cp -f lgraph.spec $RPM_BUILD/SPECS

cd $RPM_BUILD/SPECS && rpmbuild -bb lgraph.spec
echo "$RPM_BUILD/RPMS/x86_64/ generated"
