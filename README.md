# TuGraph

![Build Status](https://github.com/Tugraph-db/Tugraph-db/actions/workflows/ci.yml/badge.svg)

## 0. Introduction
TuGraph is an efficient graph database that supports high data volume, low latency lookup and fast graph analytics.

Functionalities:

- Labeled property graph model
- Full ACID support with serializable transactions
- Graph analytics algorithms embedded with graph computing framework
- OpenCypher query language
- Primary/Secondary Index in vertex and edge
- Full-Text Index support
- Graph visualization with web interface
- Java/Python/C++ clients available
- RESTful and RPC API support
- Stored procedure with C++/Python API
- Efficient development of new graph algorithms with Traversal API
- Multi-graph support
- Online/offline backup/restore
- Various data source support, including CSV/JSON/MySQL/Hive, etc. (integrated DataX)
- Monitoring System (integrated Prometheus and Grafana)
- Job Management System
- Unit Test and Integration Test

Performance and scalability:

- Supports up to tens of terabytes
- Visit millions of vertices per second
- Fast bulk import
- LDBC SNB world record holder (2022/9/1 https://ldbcouncil.org/benchmarks/snb/)

## 1. Quick Start

An easy way to start is using docker to set up, which can be found in DockerHub, named `tugraph/tugraph-db-[os]:[tugraph version]`,
for example, `tugraph/tugraph-db-centos7:3.3.0`.

For more details, please refer to [doc-zh/1.guide/3.quick-start.md]

## 2. Install

### 2.1 Compile the project with GCC on UNIX:
1. `deps/build_deps.sh` or `SKIP_WEB=1 deps/build_deps.sh` to skip building web interface
2. `cmake .. -DOURSYSTEM=centos` or `cmake .. -DOURSYSTEM=ubuntu`
3. If support shell lgraph_cypher, use `-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1`
4. `make`
5. `make package` or `cpack --config CPackConfig.cmake`

### 2.2 Compile the project with Clang on macOS:
1. `deps/build_deps.sh` or `SKIP_WEB=1 deps/build_deps.sh` to skip building web interface
2. `cmake ..`
3. `make`

### 2.3 Release Version:
1. Use gcc-5.4.0 or gcc-7.5.0
2. Use CMAKE_BUILD_TYPE `Release`
3. Check the package's directory tree (especially `include`)
4. Make sure the front-end is updated

## 3. Develop

We have prepared environment docker images for compiling in DockerHub, named `tugraph/tugraph-env-[os]:[env version]`, 
for example, `tugraph/tugraph-env-centos7:1.1.0`, which can help developers get started easily.

For more details, please refer to the docs in [doc-zh]

## 4. Contact

Email: tugraph@service.alipay.com

Slack(English):
[TuGraph.slack](https://tugraph.slack.com/)

DingTalk Group(Simplified Chinese):

![alert](./doc/images/dingtalk.png)

WeChat Official Account(Simplified Chinese):

![alert](./doc/images/wechat.png)

