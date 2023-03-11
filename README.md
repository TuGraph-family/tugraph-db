# TuGraph

![Build Status](https://github.com/Tugraph-db/Tugraph-db/actions/workflows/ci.yml/badge.svg)
![Coverage Badge](https://codecov.io/github/TuGraph-db/tugraph-db/branch/master/graph/badge.svg)

:mega: **TuGraph-db [Free Trial](https://computenest.console.aliyun.com/user/cn-hangzhou/serviceInstanceCreate?ServiceId=service-7b50ea3d20e643da95bf&ServiceVersion=1&isTrial=true) on Aliyun with [Guide](https://aliyun-computenest.github.io/quickstart-tugraph/)**

## 0. Introduction
TuGraph is an efficient graph database that supports high data volume, low latency lookup and fast graph analytics.

Functionalities:

- Labeled property graph model
- Full ACID support with serializable transactions
- Graph analytics algorithms embedded with graph computing framework
- Full-Text/Primary/Secondary Index support
- OpenCypher query API
- Stored procedure with C++/Python API

Performance and scalability:

- LDBC SNB world record holder (2022/9/1 https://ldbcouncil.org/benchmarks/snb/)
- Supports up to tens of terabytes
- Visit millions of vertices per second
- Fast bulk import

Welcome to our [website](https://www.tugraph.org)

## 1. Quick Start

An easy way to start is using docker to set up, which can be found in DockerHub, named `tugraph/tugraph-runtime-[os]:[tugraph version]`,
for example, `tugraph/tugraph-runtime-centos7:3.3.0`.

For more details, please refer to [quick start doc](doc/zh-CN/1.guide/3.quick-start.md).

## 2. Build from Source

It's recommended to build TuGraph in linux system, and docker environment is a good choice. If you want to setup a new environment, please refer to [Dockerfile](ci/images).

Here are steps to compile TuGraph:
1. `deps/build_deps.sh` or `SKIP_WEB=1 deps/build_deps.sh` to skip building web interface
2. `cmake .. -DOURSYSTEM=centos` or `cmake .. -DOURSYSTEM=ubuntu`
3. If support shell lgraph_cypher, use `-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1`
4. `make`
5. `make package` or `cpack --config CPackConfig.cmake`

## 3. Develop

We have prepared environment docker images for compiling in DockerHub, named `tugraph/tugraph-compile-[os]:[compile version]`, 
for example, `tugraph/tugraph-compile-centos7:1.1.0`, which can help developers get started easily.

We have a [roadmap](doc/zh-CN/6.community/3.roadmap.md) to help you understand TuGraph.

To contribute, please read [doc](doc/zh-CN/6.community/1.contributing.md)

NOTICE: If you want to contribute code, you should sign a [cla doc](https://cla-assistant.io/TuGraph-db/tugraph-db).

## 4. Contact

Email: tugraph@service.alipay.com

Official Website: [www.tugraph.org](https://www.tugraph.org)

Github Issue (For bug report and feature request)
[Issue](https://github.com/TuGraph-db/tugraph-db/issues)

Github Discussions (For general technical discussion)
[Discussions](https://github.com/TuGraph-db/tugraph-db/discussions)

Slack (For developer quick communication):
[TuGraph.slack](https://join.slack.com/t/tugraph/shared_invite/zt-1hha8nuli-bqdkwn~w4zH1vlk0QvqIfg)

DingTalk Group (For business and market communication):

![alert](./doc/images/dingtalk.png)

WeChat Official Account (For announcement)

![alert](./doc/images/wechat.png)

