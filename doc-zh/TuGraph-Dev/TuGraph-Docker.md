# TuGraph Docker手册

Version: 3.3.0

2022/07/19

蚂蚁集团

---

# 1. 简介

内容仅针对开发人员，如需了解使用方法请参考 `doc/TuGraph-Start/TuGraph-QuickStart.md`。
本文档介绍TuGraph Compile及TuGraph Release的Docker镜像的创建、下载。

- TuGraph Compile Image：提供编译环境，可以用于TuGraph的编译。
- TuGraph Release Image：基于"TuGraph Compile Image"，附带TuGraph库和可执行文件。

TODO: 精简 TuGraph Release Image，删除仅编译需要的库环境。

# 2. 现有Docker Image

### 镜像下载方式

镜像下载需要权限申请，请咨询主管。
镜像仓库地址：`https://docker.alibaba-inc.com/`。
镜像下载更新方法如下：

```bash
docker login --username=${username} reg.docker.alibaba-inc.com
docker pull reg.docker.alibaba-inc.com/fma/${image_name}:${image_tag}
```

### TuGraph Compile Image

提供编译环境，可以用于TuGraph的编译。

#### 命名规范

reg.docker.alibaba-inc.com/fma/tugraph-env-[os name & version]:[tugraph env version]

#### 镜像列表

- reg.docker.alibaba-inc.com/fma/tugraph-env-ubuntu16.04:1.0.2
- reg.docker.alibaba-inc.com/fma/tugraph-env-ubuntu18.04:1.0.2
- reg.docker.alibaba-inc.com/fma/tugraph-env-centos7.3:1.0.1

### TuGraph Release Image

基于"TuGraph Compile Image"，附带TuGraph库和可执行文件，安装目录为`/usr/local`。

#### 命名规范

reg.docker.alibaba-inc.com/fma/tugraph-[os name & version]:[tugraph version]-[tugraph commit id]

#### 镜像列表

- reg.docker.alibaba-inc.com/fma/tugraph-ubuntu16.04:3.0.0-5b39a12d
- reg.docker.alibaba-inc.com/fma/tugraph-ubuntu18.04:3.0.0-5b39a12d
- reg.docker.alibaba-inc.com/fma/tugraph-centos7.3:3.0.0-5b39a12d

# 3. 创建Docker镜像

注意创建镜像需要下载依赖，所以因为网络问题会导致创建较慢或者创建失败。

后续补充通过本地导入依赖的方式。

### 创建"TuGraph Compile Image"

不同操作系统版本的Dockerfile在 `.circleci/images/`目录下，目前支持 `ubuntu16.04`、`ubuntu18.04`、`centos7.3`。

```bash
cd .circleci/images/${os_version}
docker build -t reg.docker.alibaba-inc.com/fma/${image_name}:${image_tag} .
```

示例如下

```bash
cd .circleci/images/centos7.3
docker build -t reg.docker.alibaba-inc.com/fma/tugraph-env-centos7.3:1.0.1 .
```

### 创建"TuGraph Release Image"

创建TuGraph Docker镜像需要指定：

- TuGraphPath: TuGraph文件夹路径，来自(https://code.alipay.com/fma/TuGraph)。
- TuGraphEnvDockerImage: “TuGraph Compile Image”以及标签。
- TuGraphDockerImage: 输出的Docker镜像及标签。
- DataX_Path: DataX路径。

```bash
cd .circleci/images/TuGraph
bash build.sh ${TuGraphPath} ${TuGraphEnvDockerImage} ${TuGraphDockerImage} ${DataX_Path}
```

示例如下

```bash
cd .circleci/images/TuGraph
bash build.sh /data/TuGraph \
    reg.docker.alibaba-inc.com/fma/tugraph-env-ubuntu16.04:1.0.2 \
    reg.docker.alibaba-inc.com/fma/tugraph-ubuntu16.04:3.0.0
```

### 上传镜像

注意不要覆盖非tag为 `latest`的镜像。

```
docker login --username= reg.docker.alibaba-inc.com
docker push reg.docker.alibaba-inc.com/fma/${image_name}:${image_tag}
```

### 修改镜像名称

```bash
docker tag ${image_name}:${image_tag} reg.docker.alibaba-inc.com/tugraph/tugraph:3.3.0
```

### 导出镜像

```bash
docker save ${image_name}:${image_tag} | gzip > lgraph_latest.tar.gz
```

# 4. 修改日志

### TuGraph Compile Image

- reg.docker.alibaba-inc.com/fma/tugraph-env-ubuntu16.04

  - 1.0.2 (2022-01-14)
    1. 将Python3.5.2更新到Python3.6.9。
    2. 清理多余的boost1.68文件。
    3. 加入node.js。
  - 1.0.1 (2022-01-14)
    1. 基础的TuGraph编译环境。
- reg.docker.alibaba-inc.com/fma/tugraph-env-ubuntu18.04

  - 1.0.2 (2022-02-25)
    1. 添加JDK8和Maven-3.6.0。

  - 1.0.1 (2022-01-14)
    1. 基础的TuGraph编译环境。
    2. 使用Python3.6.9。
- reg.docker.alibaba-inc.com/fma/tugraph-env-centos7.3

  - 1.0.1 (2022-01-14)
    1. 基础的TuGraph编译环境。
    2. 使用Python3.6.9。
- reg.docker.alibaba-inc.com/fma/tugraph-env-centos7.3

  - 1.0.2 (2022-04-22)
    1. 添加openjdk-1.8环境
- reg.docker.alibaba-inc.com/fma/tugraph-env-centos7.3

  - 1.0.3 (2022-05-12)
    1. 添加libcurl依赖库
    2. 添加prometheus依赖库
    3. 添加grafana依赖库
    4. 安装pytest集成测试环境

### TuGraph Release Image

- reg.docker.alibaba-inc.com/fma/tugraph-ubuntu16.04:3.0.0-5b39a12d

  - 3.0.0-5b39a12d (2022-01-14)
- reg.docker.alibaba-inc.com/fma/tugraph-ubuntu18.04:3.0.0-5b39a12d
  - 3.1.0-b3aa17d0 (2022-02-25)
    1. 加入DataX (支持MySQL、HDFS、KafKa)。
  - 3.0.0-5b39a12d (2022-01-16)
- reg.docker.alibaba-inc.com/fma/tugraph-centos7.3:3.0.0-5b39a12d

  - 3.0.0-5b39a12d (2022-01-16)
