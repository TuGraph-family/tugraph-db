#!/bin/bash

mkdir -p rpc_client/java/TuGraphOGMTest/deps 
DEPSDIR=`pwd`/rpc_client/java/TuGraphOGMTest/deps
echo $DEPSDIR
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/tugraph-rpc-driver-0.1.0-SNAPSHOT.jar  -O ${DEPSDIR}/tugraph-rpc-driver-0.1.0-SNAPSHOT.jar
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/neo4j-ogm-core-0.1.0-SNAPSHOT.jar      -O ${DEPSDIR}/neo4j-ogm-core-0.1.0-SNAPSHOT.jar  
wget https://tugraph-web.oss-cn-beijing.aliyuncs.com/OGM/neo4j-ogm-api-0.1.0-SNAPSHOT.jar       -O ${DEPSDIR}/neo4j-ogm-api-0.1.0-SNAPSHOT.jar

mvn install:install-file -Dfile=${DEPSDIR}/neo4j-ogm-core-0.1.0-SNAPSHOT.jar -DgroupId=org.neo4j -DartifactId=neo4j-ogm-core -Dversion=0.1.0-SNAPSHOT  -Dpackaging=jar
mvn install:install-file -Dfile=${DEPSDIR}/tugraph-rpc-driver-0.1.0-SNAPSHOT.jar -DgroupId=org.neo4j -DartifactId=tugraph-rpc-driver -Dversion=0.1.0-SNAPSHOT  -Dpackaging=jar
mvn install:install-file -Dfile=${DEPSDIR}/neo4j-ogm-api-0.1.0-SNAPSHOT.jar -DgroupId=org.neo4j -DartifactId=neo4j-ogm-api -Dversion=0.1.0-SNAPSHOT  -Dpackaging=jar

mvn clean -f rpc_client/java/TuGraphOGMTest/pom.xml install
