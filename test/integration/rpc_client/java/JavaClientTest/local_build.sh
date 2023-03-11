#!/usr/bin/env bash

#build TuGraphRpcClient and OGM
mvn clean -f  ../../deps/tugraph-client-java/pom.xml install -DskipTests

#build JavaClientTest
mvn clean -f rpc_client/java/JavaClientTest/pom.xml install -DskipTests
