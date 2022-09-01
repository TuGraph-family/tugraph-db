# TuGraph

## 0. Introduction
TuGraph is an efficient graph database that supports high data volume, low latency lookup and fast graph analytics.

Functionalities:

- Labeled property graph model
- Full ACID support with serializable transactions
- Graph analytics algorithms embedded with graph computing framework
- OpenCypher query language
- Primary / Secondary Index in vertex and edge
- Fulltext Index support
- Graph visualization with web
- Java / Python / C++ client available
- RESTful and RPC API support
- Stored procedure with C++/Python API
- Efficient development of new graph algorithms with Traversal API
- Multi-graph support
- Online / offline backup / restore
- Various Data source support, including csv / json / MySQL / Hive, etc (integrated DataX)
- Monitor System (integrated Prometheus and Grafana)
- Job Management System
- Unit Test and Integration Test

Performance and scalability:

- Supports up to tens of terabytes
- Visit millions of vertices per second
- Fast bulk import
- LDBC SNB world record holder (2022/9/1 https://ldbcouncil.org/benchmarks/snb/)

## 1. Quick Start

Please refer [doc-zh/1.Guide/3.quick-start.md]

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
4. Make sure the front-end updated

## 3. Develop

Please refer docs in [doc-zh]

## 4. Contact

Email: tugraph@service.alipay.com

DingTalk Group:

![alert](./doc/images/dingtalk.png)

WeChat Official Accounts:

![alert](./doc/images/wechat.png)

