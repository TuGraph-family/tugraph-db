# TuGraph

[![Release](https://shields.io/github/v/release/tugraph-family/tugraph-db.svg?logo=stackblitz&label=Version&color=red)](https://github.com/TuGraph-family/tugraph-db/releases)
[![UT&&IT](https://github.com/TuGraph-family/tugraph-db/actions/workflows/ci.yml/badge.svg)](https://github.com/TuGraph-family/tugraph-db/actions/workflows/ci.yml)
[![Documentation Status](https://readthedocs.org/projects/tugraph-db/badge/?version=latest)](https://tugraph-db.readthedocs.io/en/latest/?badge=latest)
[![Commit](https://badgen.net/github/last-commit/tugraph-family/tugraph-db/master?icon=git&label=Commit)](https://github.com/TuGraph-family/tugraph-db/commits/master)
[![codecov](https://codecov.io/gh/TuGraph-family/tugraph-db/branch/master/graph/badge.svg?token=JH78ARWZAQ)](https://codecov.io/gh/TuGraph-family/tugraph-db)

[![Star](https://shields.io/github/stars/tugraph-family/tugraph-db?logo=startrek&label=Star&color=yellow)](https://github.com/TuGraph-family/tugraph-db/stargazers)
[![Fork](https://shields.io/github/forks/tugraph-family/tugraph-db?logo=forgejo&label=Fork&color=orange)](https://github.com/TuGraph-family/tugraph-db/forks)
[![Contributor](https://shields.io/github/contributors/tugraph-family/tugraph-db?logo=actigraph&label=Contributor&color=abcdef)](https://github.com/TuGraph-family/tugraph-db/contributors)
[![Docker](https://shields.io/docker/pulls/tugraph/tugraph-runtime-centos7?logo=docker&label=Docker&color=blue)](https://hub.docker.com/r/tugraph/tugraph-runtime-centos7/tags)
[![License](https://shields.io/github/license/tugraph-family/tugraph-db?logo=apache&label=License&color=blue)](https://www.apache.org/licenses/LICENSE-2.0.html)

[![EN](https://shields.io/badge/Docs-English-blue?logo=readme)](https://tugraph-db.readthedocs.io/en/latest)
[![CN](https://shields.io/badge/Docs-中文-blue?logo=readme)](https://tugraph-db.readthedocs.io/zh-cn/latest)

[[中文版]](README_CN.md)

:mega: **TuGraph-db [Free Trial](https://computenest.console.aliyun.com/user/cn-hangzhou/serviceInstanceCreate?ServiceId=service-7b50ea3d20e643da95bf&ServiceVersion=1&isTrial=true) on Aliyun with [Guide](https://aliyun-computenest.github.io/quickstart-tugraph/)**.

## 1. Introduction
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

You can find TuGraph's doc by [link](https://tugraph-db.readthedocs.io/en/latest), and welcome to our [website](https://www.tugraph.org).

## 2. Quick Start

An easy way to start is using docker to set up, which can be found in [DockerHub](https://hub.docker.com/u/tugraph), named `tugraph/tugraph-runtime-[os]:[tugraph version]`,
for example, `tugraph/tugraph-runtime-centos7:3.3.0`.

For more details, please refer to [quick start doc](docs/en-US/source/3.quick-start/1.preparation.md).

## 3. Build from Source

It's recommended to build TuGraph in linux system, and docker environment is a good choice. If you want to setup a new environment, please refer to [Dockerfile](ci/images).

Here are steps to compile TuGraph:
1. run `deps/build_deps.sh` to build tugraph-web if you need. Skip this step otherwise.
2. `cmake .. -DOURSYSTEM=centos` or `cmake .. -DOURSYSTEM=ubuntu`
3. If support shell lgraph_cypher, use `-DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1`
4. `make`
5. `make package` or `cpack --config CPackConfig.cmake`

Example:
`tugraph/tugraph-compile-centos7`Docker environment

```bash
$ git clone --recursive https://github.com/TuGraph-family/tugraph-db.git
$ cd tugraph-db
$ deps/build_deps.sh
$ mkdir build && cd build
$ cmake .. -DOURSYSTEM=centos -DENABLE_PREDOWNLOAD_DEPENDS_PACKAGE=1
$ make
$ make package
```

## 4. Develop

We have prepared environment docker images for compiling in DockerHub, named `tugraph/tugraph-compile-[os]:[compile version]`, 
for example, `tugraph/tugraph-compile-centos7:1.1.0`, which can help developers get started easily.

We have a [roadmap](docs/en-US/source/12.contributor-manual/5.roadmap.md) to help you understand TuGraph.

To contribute, please read [doc](docs/en-US/source/12.contributor-manual/1.contributing.md).

NOTICE: If you want to contribute code, you should sign a [cla doc](https://cla-assistant.io/TuGraph-db/tugraph-db).

## 5. Partners

<table cellspacing="0" cellpadding="0">
  <tr align="center">
    <td height="80"><a href="https://github.com/CGCL-codes/YiTu"><img src="docs/images/partners/hust.png" width="300" alt="HUST" /></a></td>
    <td height="80"><a href="http://kw.fudan.edu.cn/"><img src="docs/images/partners/fu.png" width="300" alt="FU" /></a></td>
    <td height="80"><img src="docs/images/partners/zju.png" width="300" alt="ZJU" /></td>
  </tr>
  <tr align="center">
    <td height="80"><a href="http://www.whaleops.com/"><img src="docs/images/partners/whaleops.png" width="300" alt="WhaleOps" /></a></td>
    <td height="80"><a href="https://github.com/oceanbase/oceanbase"><img src="docs/images/partners/oceanbase.png" width="300" alt="OceanBase" /></a></td>
    <td height="80"><a href="https://github.com/secretflow/secretflow"><img src="docs/images/partners/secretflow.png" width="300" alt="SecretFlow" /></a></td>
  </tr>
</table>

## 6. Contact

Official Website: [www.tugraph.org](https://www.tugraph.org)

Slack (For developer quick communication):
[TuGraph.slack](https://join.slack.com/t/tugraph/shared_invite/zt-1hha8nuli-bqdkwn~w4zH1vlk0QvqIfg)

Contact us via dingtalk, wechat, email and telephone:
![contacts](./docs/images/contact-en.png)



