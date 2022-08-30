#!/usr/bin/env bash

PROTO_DIR=`pwd`/../../../protobuf

mkdir -p `pwd`/rpc-client/src/main/proto/
ln -sf ${PROTO_DIR}/ha.proto `pwd`/rpc-client/src/main/proto/lgraph.proto

mvn clean install -DskipTests

