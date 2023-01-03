#!/usr/bin/env bash

#build TuGraphRpcClient
mvn clean -f  ../../src/client/java/TuGraphRpcClient/pom.xml install -DskipTests

#build JavaClientTest
mvn clean -f rpc_client/java/JavaClientTest/pom.xml install -DskipTests
