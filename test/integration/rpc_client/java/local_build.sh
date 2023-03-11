#!/bin/bash

# Env config
export MAVEN_OPTS=-Xss10m
mkdir -p ~/.m2
echo "<settings>
  <mirrors>
    <mirror>
      <id>alimaven</id>
      <mirrorOf>central</mirrorOf>
      <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
    </mirror>
  </mirrors>
</settings>" > ~/.m2/settings.xml

#build TuGraphDbRpcClient
mvn clean -f  ../../deps/tugraph-db-client-java/pom.xml install -DskipTests

#build JavaClientTest
mvn clean -f rpc_client/java/JavaClientTest/pom.xml install -DskipTests

#build OgmTest
mvn clean -f rpc_client/java/TuGraphOGMTest/pom.xml install
