# TuGraph 示例

## 1 简介

TuGraph 是蚂蚁集团自主研发的大规模图计算系统，提供图数据库引擎和图分析引擎。其主要特点是大数据量存储和计算，高吞吐率，以及灵活的 API，同时支持高效的在线事务处理（OLTP）和在线分析处理（OLAP）。 LightGraph、GeaGraph是TuGraph的曾用名。

主要功能特征包括：

- 支持属性图模型
- 原生图存储及处理
- 完全的ACID事务支持
- 支持OpenCypher图查询语言
- 支持原生的Core API和Traversal API
- 支持REST和RPC接口
- 支持CSV、JSON、MySQL等多数据源导入导出
- 支持可视化图交互
- 支持命令行交互
- 内置用户权限控制、操作审计
- 支持任务和日志的监控管理
- 原生适配PandaGraph图分析引擎
- 集成DGL图神经网络系统

性能及可扩展性特征包括：

- 支持TB级大容量
- 吞吐率高达千万顶点每秒
- 面向读优化的存储引擎
- 支持高可用模式
- 支持离线备份恢复
- 在线热备份
- 高性能批量导入导出

## 2 快速上手

见QuickStart文档。

## 3 基本功能

### 3.1 RPC Client
#### 3.1.1 概述
RPC Client是对cpp语言rpc客户端的简单封装，每次执行时会创建一条到lgraph_server的链接用于发送请求数据以及接收响应结果，执行完毕后进程退出前会断开链接
#### 3.1.2 编译
在代码目录demo/CppRpcClientDemo目录下,执行下列命令 ,成功后将会看到可执行文件clientdemo
```bash
mkdir build && cd build && cmake ../ && make
```
#### 3.1.3 运行
先启动lgraph_server，确保rpc端口处于打开状态。

clientdemo程序接收参数如下：
        -h             show this usage
        -i --ip        ip for graph server
        -p --port      port for graph server
        -g --graph     graph name
        -u --user      user name
        --password     user password
        -c --cypher    cypher to query
举例如下
```bash
./clientdemo -i 127.0.0.1 -p 9090 -u admin --password 73@TuGraph -g default -c "MATCH (n) RETURN n LIMIT 100"
```
### 3.2 Python RPC Client
#### 3.2.1 概述
Python RPC Client是对python语言rpc客户端的简单封装，每次执行时会创建一条到lgraph_server的链接用于发送请求数据以及接收响应结果，执行完毕后进程退出前会断开链接
#### 3.2.2 运行
需要依赖编译生成的python_client.so库，将python_client.so与client_python.py放在同一目录下
先启动lgraph_server，确保rpc端口处于打开状态。

clientdemo程序接收参数如下：
-h             show this usage
-i --ip        ip for graph server
-p --port      port for graph server
-g --graph     graph name
-u --user      user name
--password     user password
-c --cypher    cypher to query
举例如下
```bash
python3 client_python.py -i 127.0.0.1 -p 9090 -u admin --password 73@TuGraph -g default -c "MATCH (n) RETURN n LIMIT 100"
```
## 4 集成工具

### 4.1 DataX 导入导出工具
#### 4.1.1 概述
DataX 支持 TuGraph 和 MySQL、SQL Server、Oracle、PostgreSQL、HDFS、Hive、HBase、OTS、ODPS、Kafka 等各种异构数据源的数据导入导出。
#### 4.1.2 运行
以MySQL和为例，先在数据源中准备数据
```bash
mysql> source mysql_data.sql
```
然后启动TuGraph服务，使用DataX将数据从数据源导入到TuGraph中
```bash
bash ./import_from_mysql.sh
```

### 4.2 DGL图神经网络系统

#### 4.2.1 概述

DGL是一个开源的高性能图神经网络框架，能兼容PyTorch、Apache MXNet以及TensorFlow等主流机器学习库，并提供了多种常见的图学习算法。

#### 4.2.2 环境配置

使用pip3安装pytorch和dgl：

```shell script
pip3 install torch
pip3 install dgl -f https://data.dgl.ai/wheels/repo.html
```

#### 4.2.3 图数据导入

在demo/DGLDemo目录下执行以下命令，将data_cora数据导入到TuGraph数据库中

```shell script
lgraph_import -c data_cora/import.json -d db_cora --overwrite 1
```

#### 4.2.4 运行

在demo/DGLDemo目录下执行以下命令，运行gcn程序。

```shell script
python3 gcn.py --db_path db_cora --password 73@TuGraph
```


## 5 管理功能

## 6 高可用