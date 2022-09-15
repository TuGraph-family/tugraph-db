# TuGraph Docker手册

# 1. 简介

内容仅针对开发人员，本文档介绍TuGraph Compile及TuGraph Runtime的Docker镜像的创建、下载。

- TuGraph Compile Image：提供编译环境，可以用于TuGraph的编译，测试；
- TuGraph Runtime Image：提供二进制可运行环境，附带TuGraph库和可执行文件。

# 2. 现有Docker Image

### 镜像下载方式

镜像托管在DockerHub，可直接下载使用。

### TuGraph Compile Image

提供编译环境，可以用于TuGraph的编译。

#### 命名规范

tugraph/tugraph-compile-[os name & version]:[tugraph compile version]

#### 镜像列表

- tugraph/tugraph-compile-centos7:1.1.0

### TuGraph Runtime Image

提供二进制可运行环境，附带TuGraph库和可执行文件。

#### 命名规范

tugraph/tugraph-runtime-[os name & version]:[tugraph-runtime version]

#### 镜像列表

- tugraph/tugraph-runtime-centos7:3.3.0

# 3. 创建Docker镜像

注意创建镜像需要下载依赖，所以因为网络问题会导致创建较慢或者创建失败。

后续补充通过本地导入依赖的方式。

### 创建"TuGraph Compile Image"

不同操作系统版本的Dockerfile在 `ci/images/`目录下，根据需要选择compile版本，目前提供centos7版本

```bash
cd ci/images/${version}
docker build -f compile/centos-7-Dockerfile -t tugraph/${image_name}:${image_tag} .
```

示例如下

```bash
cd ci/images/compile
docker build -f centos-7-Dockerfile -t tugraph/tugraph-compile-centos7:1.1.0 .
```

### 创建"TuGraph Runtime Image"

创建TuGraph Docker镜像需要指定：

- TuGraphPath: tugraph-db文件夹路径。
- CompileDockerImage: “TuGraph Compile Image”以及标签。
- RuntimeDockerImage: 输出的Runtime Docker镜像及标签。
- DataX_Path: DataX路径。

```bash
cd ci/images
bash build.sh ${TuGraphPath} ${CompileDockerImage} ${RuntimeDockerImage} ${DataX_Path}
```

示例如下

```bash
cd ci/images/runtime
bash build.sh /data/TuGraph \
    tugraph/tugraph-env-centos7:1.1.0 \
    tugraph/tugraph-db-centos7:3.3.0
```

### 上传镜像

注意不要覆盖非tag为 `latest`的镜像。

### 修改镜像名称

```bash
docker tag ${image_name}:${image_tag} tugraph/tugraph-runtime-centos7:3.3.0
```

### 导出镜像

```bash
docker save ${image_name}:${image_tag} | gzip > lgraph_latest.tar.gz
```

# 4. 修改日志

### TuGraph Compile Image

- first update

### TuGraph Release Image

- first update
