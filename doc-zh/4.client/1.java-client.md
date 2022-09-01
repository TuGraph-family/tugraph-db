# TuGraph Java SDK

## **配置Maven**

**配置dev仓库**

```xml
<mirror>
    <id>alimaven</id>
    <name>aliyun maven</name>
    <url>http://mvn.dev.alipay.net/nexus/content/groups/public/</url>
    <mirrorOf>central</mirrorOf>
</mirror>
```
**使用fat jar**
编译时候需要添加"MAVEN_OPTS=-Xss300M"参数
示例：MAVEN_OPTS=-Xss300M mvn package
```xml
<dependency>
    <groupId>com.antgroup</groupId>
    <artifactId>tugraph-java-rpc-client</artifactId>
    <version>3.1.0</version>
</dependency>
```
**使用示例**

### **实例化client对象**
引入依赖并实例化
```java
import com.alipay.tugraph.TuGraphRpcClient;
TuGraphRpcClient client = new TuGraphRpcClient(url, user, password);
```
```comment
public TuGraphRpcClient(String url, String user, String pass)
@param url: tugraph host looks like list://ip:port
@param user: login user name 
@param password: login password
```

### **调用cypher**
```java
    String res = client.callCypher("CALL db.edgeLabels()", "default", 10);
    log.info("db.edgeLabels() : " + res);
```
```comment
    @param cypher: inquire statement.
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of cypher query execution
    public String callCypher(String cypher, String graph, double timeout) 
```
### **调用存储过程**
```java
    String result = client.callPlugin("CPP", "khop", kHopParamGen(), 1000, false, "default", 1000);
    log.info("testCallPlugin : " + result);
```
```comment
    @param pluginType: the plugin type, currently supported CPP and PY
    @param pluginName: plugin name
    @param param: the execution parameters
    @param pluginTimeOut: Maximum execution time, overruns will be interrupted
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of plugin execution
    public String callPlugin(String pluginType, String pluginName, String param, double pluginTimeOut,
            boolean inProcess, String graph, double timeout)
```
### **加载存储过程**
```java
    String result = client.loadPlugin("./test/plugin/khop.so", "CPP", "khop", "SO", "test loadplugin", true, "default", 1000);
    log.info("loadPlugin : " + result);
```
```comment
    @param sourceFile: the source_file contain plugin code
    @param pluginType: the plugin type, currently supported CPP and PY
    @param pluginName: plugin name
    @param codeType: code type, currently supported PY, SO, CPP, ZIP
    @param pluginDescription: plugin description
    @param readOnly: plugin is read only or not
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of plugin execution
    public boolean loadPlugin(String sourceFile, String pluginType, String pluginName, String codeType,
            String pluginDescription, boolean readOnly, String graph, double timeout) throws IOException
```
### **从字节流中导入schema**
```java
    boolean ret = client.importSchemaFromContent(schema, "default", 1000);
    log.info("importSchemaFromContent : " + ret);
```
```comment
    @param schema: the schema to be imported
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of import schema
    public boolean importSchemaFromContent(String schema, String graph, double timeout) throws UnsupportedEncodingException 
```
### **从字节流中导入点边数据**
```java
    boolean ret = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000);
    log.info("importDataFromContent : " + ret);
```
```comment
    @param desc: data format description
    @param data: the data to be imported
    @param delimiter: data separator
    @param continueOnError: whether to continue when importing data fails
    @param threadNums: maximum number of threads
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of import data
    public boolean importDataFromContent(String desc, String data, String delimiter, boolean continueOnError,
            int threadNums, String graph, double timeout) throws UnsupportedEncodingException
```
### **从文件中导入schema**
```java
    boolean ret = client.importSchemaFromFile("./test/data/yago.conf", "default", 1000);
    log.info("importSchemaFromFile : " + ret);
```
```comment
    @param schemaFile: the schema_file contain schema
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of import schema
    public boolean importSchemaFromFile(String schemaFile, String graph, double timeout) 
            throws UnsupportedEncodingException, IOException
```
### **从文件中导入点边数据**
```java
    boolean ret = client.importDataFromFile("./test/data/yago.conf", ",", true, 16, 0, "default", 1000000000);
    log.info("importDataFromFile : " + ret);
```
```comment
    @param confFile: data file contain format description and data
    @param delimiter: data separator
    @param continueOnError: whether to continue when importing data fails
    @param threadNums: maximum number of threads
    @param skipPackages: skip packages number
    @param graph: the graph to query.
    @param timeout: Maximum execution time, overruns will be interrupted
    @return: the result of import data
    public boolean importDataFromFile(String confFile, String delimiter, boolean continueOnError, int threadNums,
            int skipPackages, String graph, double timeout) throws IOException, UnsupportedEncodingException
```