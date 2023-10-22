# C++客户端

> 此文档主要是TuGraph C++ SDK的使用说明。

## 1.概述
C++ Client 能够使用 RPC 连接lgraph_server，进行数据导入、执行存储过程、调用Cypher等操作。

## 2.使用示例

### 2.1.实例化client对象
引入依赖并实例化

#### 2.1.1.实例化单节点client对象
当以单节点模式启动server时，client按照如下格式进行实例化
``` C++
RpcClient client("127.0.0.1:19099", "admin", "73@TuGraph");
```
```
RpcClient(const std::string& url, const std::string& user, const std::string& password);
@param url: tugraph host looks like ip:port
@param user: login user name
@param password: login password
```

#### 2.1.2.实例化HA集群直接连接client对象
当服务器上部署的HA集群可以使用ha_conf中配置的网址直接连接时，client按照如下格式进行实例化
``` C++
RpcClient client("127.0.0.1:19099", "admin", "73@TuGraph");
```
```
RpcClient(const std::string& url, const std::string& user, const std::string& password);
@param url: tugraph host looks like ip:port
@param user: login user name 
@param password: login password
```
用户只需要传入HA集群中的任意一个节点的url即可，client会根据server端返回的查询信息自动维护连接池，在HA集群横向扩容时
也不需要手动重启client。

#### 2.1.3.实例化HA集群间接连接client对象
当服务器上部署的HA集群不能使用ha_conf中配置的网址直接连接而必须使用间接网址（如阿里云公网网址）连接时，
client按照如下格式进行实例化。
```java
std::vector<std::string> urls = {"189.33.97.23:9091", "189.33.97.24:9091", "189.33.97.25:9091"};
TuGraphDbRpcClient client = new TuGraphDbRpcClient(urls, "admin", "73@TuGraph");
```
```
RpcClient(std::vector<std::string>& urls, std::string user, std::string password)
@param urls: tugraph host list
@param user: login user name
@param password: login password
```
因为用户连接的网址和server启动时配置的信息不同，不能通过向集群发请求的方式自动更新client连接池，所以需要在启动
client时手动传入所有集群中节点的网址，并在集群节点变更时手动重启client。

### 2.2.调用cypher
```C++
    std::string str;
    bool ret = client.CallCypher(str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");

```
```
    bool CallCypher(std::string& result, const std::string& cypher,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0, const std::string& url = "");
    @param [out] result      The result.
    @param [in]  cypher      inquire statement.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @param [in]  url         (Optional) Node address of calling cypher.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，在HA模式下的client中，通过指定url参数可以定向向某个server发送读请求。

### 2.3.向leader发送cypher请求
```C++
    std::string str;
    bool ret = client.CallCypherToLeader(str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
```
```
    bool CallCypherToLeader(std::string& result, const std::string& cypher,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0);
    @param [out] result      The result.
    @param [in]  cypher      inquire statement.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口只支持在HA模式下使用，在HA模式下的client中，为防止向未同步数据的follower发送请求，
用户可以直接向leader发送请求，leader由集群选出。

### 2.4.调用GQL
```C++
    std::string str;
    bool ret = client.CallGql(str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
```
```
    bool CallGql(std::string& result, const std::string& gql,
                    const std::string& graph = "default", bool json_format = true,
                    double timeout = 0, const std::string& url = "");
    @param [out] result      The result.
    @param [in]  gql         inquire statement.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @param [in]  url         (Optional) Node address of calling gql.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，在HA模式下的client中，通过指定url参数可以定向向某个server发送读请求。

### 2.5.向leader发送GQL请求
```C++
    std::string str;
    bool ret = client.CallGqlToLeader(str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
```
```
    bool CallGqlToLeader(std::string& result, const std::string& gql,
                 const std::string& graph = "default", bool json_format = true,
                 double timeout = 0);
    @param [out] result      The result.
    @param [in]  gql         inquire statement.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口只支持在HA模式下使用，在HA模式下的client中，为防止向未同步数据的follower发送请求，
用户可以直接向leader发送请求，leader由集群选出。

### 2.6.调用存储过程
```C++
    std::string str;
    bool ret = client.CallProcedure(str, "CPP", "test_plugin1", "bcefg");
```
```
    bool CallProcedure(std::string& result, const std::string& procedure_type,
                       const std::string& procedure_name, const std::string& param,
                       double procedure_time_out = 0.0, bool in_process = false,
                       const std::string& graph = "default", bool json_format = true,
                       const std::string& url = "");
    @param [out] result              The result.
    @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
    @param [in]  procedure_name      procedure name.
    @param [in]  param               the execution parameters.
    @param [in]  procedure_time_out  (Optional) Maximum execution time, overruns will be
                                     interrupted.
    @param [in]  in_process          (Optional) support in future.
    @param [in]  graph               (Optional) the graph to query.
    @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
                                     binary format.
    @param [in]  url                 (Optional) Node address of calling procedure.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用，默认以json格式直接返回存储过程的执行结果，指定jsonFormat为false可以返回字符串格式的执行结果。
其中，在HA模式下的client中，通过指定url参数可以定向向某个server发送读请求。

### 2.7.向leader调用存储过程
```C++
    std::string str;
    bool ret = client.CallProcedureToLeader(str, "CPP", "test_plugin1", "bcefg");
```
```
    bool CallProcedureToLeader(std::string& result, const std::string& procedure_type,
                       const std::string& procedure_name, const std::string& param,
                       double procedure_time_out = 0.0, bool in_process = false,
                       const std::string& graph = "default", bool json_format = true);
    @param [out] result              The result.
    @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
    @param [in]  procedure_name      procedure name.
    @param [in]  param               the execution parameters.
    @param [in]  procedure_time_out  (Optional) Maximum execution time, overruns will be
                                     interrupted.
    @param [in]  in_process          (Optional) support in future.
    @param [in]  graph               (Optional) the graph to query.
    @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
                                     binary format.
    @returns True if it succeeds, false if it fails.
```
本接口支持在HA模式下使用，默认以json格式直接返回存储过程的执行结果，指定jsonFormat为false可以返回字符串格式的执行结果。

### 2.8.加载存储过程
```C++
    std::string str;
    bool ret = client.LoadProcedure(str, code_sleep, "PY", "python_plugin1", "PY", "this is a test plugin", true)
```
```
    bool LoadProcedure(std::string& result, const std::string& source_file,
                       const std::string& procedure_type, const std::string& procedure_name,
                       const std::string& code_type, const std::string& procedure_description,
                       bool read_only, const std::string& version = "v1",
                       const std::string& graph = "default");
    @param [out] result                  The result.
    @param [in]  source_file             the source_file contain procedure code.
    @param [in]  procedure_type          the procedure type, currently supported CPP and PY.
    @param [in]  procedure_name          procedure name.
    @param [in]  code_type               code type, currently supported PY, SO, CPP, ZIP.
    @param [in]  procedure_description   procedure description.
    @param [in]  read_only               procedure is read only or not.
    @param [in]  version                 (Optional) the version of procedure.
    @param [in]  graph                   (Optional) the graph to query.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于加载存储过程是写请求，HA模式下的client只能向leader发送加载存储过程请求。

### 2.9.列举存储过程
```C++
    std::string str;
    bool ret = client.ListProcedures(str);
```
```
    bool ListProcedures(std::string& result, const std::string& procedure_type,
                        const std::string& version = "any",
                        const std::string& graph = "default", const std::string& url = "");
    @param [out] result          The result.
    @param [in]  procedure_type  (Optional) the procedure type, "" for all procedures,
                                 CPP and PY for special type.
    @param [in]  version         (Optional) the version of procedure.
    @param [in]  graph           (Optional) the graph to query.
    @param [in]  url             Node address of calling procedure.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，在HA模式下的client中，通过指定url参数可以定向向某个server发送请求。

### 2.10.删除存储过程
```C++
    std::string str;
    bool ret = client.DeleteProcedure(str, "CPP", "test_plugin1");
```
```
    bool DeleteProcedure(std::string& result, const std::string& procedure_type,
                         const std::string& procedure_name, const std::string& graph = "default");
    @param [out] result              The result.
    @param [in]  procedure_type      the procedure type, currently supported CPP and PY.
    @param [in]  procedure_name      procedure name.
    @param [in]  graph               (Optional) the graph to query.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于删除存储过程是写请求，HA模式下的client只能向leader发送删除存储过程请求。

### 2.11.从字节流中导入schema
```C++
    std::string str;
    bool ret = client.ImportSchemaFromContent(str, sImportContent["schema"]);
```
```
    bool ImportSchemaFromContent(std::string& result, const std::string& schema,
                                 const std::string& graph = "default", bool json_format = true,
                                 double timeout = 0);
    @param [out] result      The result.
    @param [in]  schema      the schema to be imported.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于导入schema是写请求，HA模式下的client只能向leader发送导入schema请求。

### 2.12.从字节流中导入点边数据
```C++
    std::string str;
    ret = client.ImportDataFromContent(str, sImportContent["person_desc"], sImportContent["person"],",");
```
```
    bool ImportDataFromContent(std::string& result, const std::string& desc,
                               const std::string& data, const std::string& delimiter,
                               bool continue_on_error = false, int thread_nums = 8,
                               const std::string& graph = "default", bool json_format = true,
                               double timeout = 0);
    @param [out] result              The result.
    @param [in]  desc                data format description.
    @param [in]  data                the data to be imported.
    @param [in]  delimiter           data separator.
    @param [in]  continue_on_error   (Optional) whether to continue when importing data fails.
    @param [in]  thread_nums         (Optional) maximum number of threads.
    @param [in]  graph               (Optional) the graph to query.
    @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
                                     binary format.
    @param [in]  timeout             (Optional) Maximum execution time, overruns will be
                                     interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于导入点边数据是写请求，HA模式下的client只能向leader发送导入点边数据请求。

### 2.13.从文件中导入schema
```C++
    std::string conf_file("./yago.conf");
    std::string str;
    ret = client.ImportSchemaFromFile(str, conf_file);
```
```
    bool ImportSchemaFromFile(std::string& result, const std::string& schema_file,
                              const std::string& graph = "default", bool json_format = true,
                              double timeout = 0);
    @param [out] result      The result.
    @param [in]  schema_file the schema_file contain schema.
    @param [in]  graph       (Optional) the graph to query.
    @param [in]  json_format (Optional) Returns the format， true is json，Otherwise, binary
                             format.
    @param [in]  timeout     (Optional) Maximum execution time, overruns will be interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于导入schema是写请求，HA模式下的client只能向leader发送导入schema请求。

### 2.14.从文件中导入点边数据
```C++
    std::string conf_file("./yago.conf");
    std::string str;
    ret = client.ImportDataFromFile(str, conf_file, ",");
```
```
    bool ImportDataFromFile(std::string& result, const std::string& conf_file,
                            const std::string& delimiter, bool continue_on_error = false,
                            int thread_nums = 8, int skip_packages = 0,
                            const std::string& graph = "default", bool json_format = true,
                            double timeout = 0);
    @param [out] result              The result.
    @param [in]  conf_file           data file contain format description and data.
    @param [in]  delimiter           data separator.
    @param [in]  continue_on_error   (Optional) whether to continue when importing data fails.
    @param [in]  thread_nums         (Optional) maximum number of threads.
    @param [in]  skip_packages       (Optional) skip packages number.
    @param [in]  graph               (Optional) the graph to query.
    @param [in]  json_format         (Optional) Returns the format， true is json，Otherwise,
                                     binary format.
    @param [in]  timeout             (Optional) Maximum execution time, overruns will be
                                     interrupted.
    @returns True if it succeeds, false if it fails.
```
本接口支持在单机模式和HA模式下使用。其中，由于导入点边数据是写请求，HA模式下的client只能向leader发送导入点边数据请求。
