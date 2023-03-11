#!/bin/bash

#build TuGraphRpcClient and OGM
mvn clean -f  ../../deps/tugraph-client-java/pom.xml install -DskipTests

mvn clean -f rpc_client/java/TuGraphOGMTest/pom.xml install
