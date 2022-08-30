#!/usr/bin/env bash

# prepare protobuf
PROTO_DIR=`pwd`/../../src/protobuf/
echo ${PROTO_DIR}
mkdir -p `pwd`/../../src/client/java/TuGraphRpcClient/rpc-client/src/main/proto/
ln -sf ${PROTO_DIR}/ha.proto `pwd`/../../src/client/java/TuGraphRpcClient/rpc-client/src/main/proto/lgraph.proto

#build TuGraphRpcClient
mvn clean -f  ../../src/client/java/TuGraphRpcClient/pom.xml install -DskipTests

#build JavaClientTest
mvn clean -f rpc_client/java/JavaClientTest/pom.xml install -DskipTests


