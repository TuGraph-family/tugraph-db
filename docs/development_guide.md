# 使用指南
## 连接 tugraph-db
### 驱动连接
tugraph-db兼容neo4j的通讯协议，因此可以使用neo4j各个语言的驱动连接tugraph-db的server。

neo4j各个语言的驱动版本推荐使用4.x的版本，不要用最新的，兼容性可能会有些问题。

[bolt driver 使用例子](../demo/Bolt)

### 终端连接
驱动是业务代码里面使用的，对于服务器上终端访问，可以使用lgraph_cli客户端。

[lgraph_cli 使用介绍](./lgraph_cli.md)

### 查询语言
语言为openCypher, tugraph-db实现了openCypher大部分常用的语法。

### Schema
数据模型是schema-free的，不需要提前创建点边schema，可以随时增加或者删除字段。

### 编译运行
[编译运行](./build_run.md)

### 内置存储过程
tugraph-db有一些内置的存储过程调用，用来进行数据库操作、schema查看、子图管理、日志管理、索引管理等。

具体请参考 [内置存储过程](./procedure.md)

### 基本的cypher语法
[基本的cypher语法](./basic_cypher.md)

### 批量upsert点边数据
[批量upsert](./upsert.md)

## 向量索引
[向量索引使用](./vector_index.md)

## 全文索引
[全文索引使用](./fulltext_index.md)