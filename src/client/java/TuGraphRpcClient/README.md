# LGraphRpcClientJava

TuGraphRpcClient using Java

```
modify BRPC_JAVA to the directory of brpc-java in local_build.sh if necessary

add mirror to /etc/maven/settings.xml(optional)
      <mirror>
        <id>alimaven</id>
        <name>aliyun maven</name>
        <url>http://maven.aliyun.com/nexus/content/groups/public/</url>
        <mirrorOf>central</mirrorOf>
      </mirror>

./local_build.sh

cd rpc-client/src/main/bin
./run_lgraph_client.sh <server-address> <thread-num> <query-num>
./run_lgraph_client.sh list://0.0.0.0:9091 1 10000 (for example)
```
