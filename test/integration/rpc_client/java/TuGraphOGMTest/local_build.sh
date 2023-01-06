#!/bin/bash

mkdir -p rpc_client/java/TuGraphOGMTest/deps 
DEPSDIR=`pwd`/rpc_client/java/TuGraphOGMTest/deps
echo $DEPSDIR
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/tugraph-rpc-driver-0.1.0.jar  -O ${DEPSDIR}/tugraph-rpc-driver-0.1.0.jar
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/tugraph-ogm-core-0.1.0.jar    -O ${DEPSDIR}/tugraph-ogm-core-0.1.0.jar  
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/tugraph-ogm-api-0.1.0.jar     -O ${DEPSDIR}/tugraph-ogm-api-0.1.0.jar

mvn install:install-file -Dfile=${DEPSDIR}/tugraph-ogm-core-0.1.0.jar -DgroupId=com.antgroup.tugraph -DartifactId=tugraph-ogm-core -Dversion=0.1.0 -Dpackaging=jar
mvn install:install-file -Dfile=${DEPSDIR}/tugraph-rpc-driver-0.1.0.jar -DgroupId=com.antgroup.tugraph -DartifactId=tugraph-rpc-driver -Dversion=0.1.0 -Dpackaging=jar
mvn install:install-file -Dfile=${DEPSDIR}/tugraph-ogm-api-0.1.0.jar -DgroupId=com.antgroup.tugraph  -DartifactId=tugraph-ogm-api -Dversion=0.1.0 -Dpackaging=jar

mvn clean -f rpc_client/java/TuGraphOGMTest/pom.xml install
