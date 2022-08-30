# TuGraph 操作手册

Version: 3.3.0

2022/07/19

蚂蚁集团

---

## 目录

- [简介](#简介)
  - [TuGraph 特性](#tugraph特性)
  - [数据模型](#数据模型)
    - [图模型](#图模型)
    - [数据类型](#数据类型)
    - [索引](#索引)
  - [软件授权](#软件授权)
- [安装](#安装)
  - [系统要求](#系统要求)
  - [通过 docker 镜像安装](#通过docker镜像安装)
  - [Ubuntu 下的安装方法](#ubuntu下的安装方法)
  - [CentOS 下的安装方法](#centos下的安装方法)
- [数据导入](#数据导入)
  - [离线全量导入](#离线全量导入)
    - [配置文件](#配置文件)
      - [目标描述](#目标描述)
      - [列映射](#列映射)
    - [离线导入示例](#离线导入示例)
  - [在线增量导入](#在线增量导入)
- [服务器配置](#服务器配置)
  - [配置参数](#配置参数)
  - [服务器配置文件](#服务器配置文件)
  - [命令行参数](#命令行参数)
- [服务启停](#服务启停)
  - [运行模式](#运行模式)
  - [运行普通进程](#运行普通进程)
  - [启动服务](#启动服务)
  - [停止服务](#停止服务)
  - [重启服务](#重启服务)
- [使用 API 访问数据库](#使用api访问数据库)
- [`lgraph_cypher` 使用说明](#lgraph_cypher-使用说明)
  - [单命令模式](#单命令模式)
    - [命令行参数:](#命令行参数-1)
    - [命令示例:](#命令示例)
  - [交互模式](#交互模式)
    - [进入 lgraph_cypher 交互模式:](#进入lgraph_cypher交互模式)
    - [command 种类与说明:](#command种类与说明)
    - [cypher 查询命令:](#cypher查询命令)
    - [辅助功能:](#辅助功能)
- [可视化工具](#可视化工具)
- [图神经网络](#图神经网络)
- [高可用模式](#高可用模式)
- [数据库管理](#数据库管理)
  - [日志信息](#日志信息)
    - [服务器日志](#服务器日志)
    - [审计日志](#审计日志)
  - [数据导出](#数据导出)
  - [数据备份](#数据备份)
  - [数据预热](#数据预热)
  - [任务管理](#任务管理)
- [常见问题](#常见问题)

---

# 简介

图数据库是按顶点和边存储数据的非关系型数据库，可用于存储复杂的数据网络模型，如社交网络和事务网络等。

TuGraph 是由蚂蚁集团开发的图数据库，本手册介绍了 TuGraph 的功能及使用方法。

## TuGraph 特性

TuGraph 是支持大数据容量、低延迟查找和快速图分析功能的高效图数据库。同时 TuGraph 也是基于磁盘的数据库，支持存储多达数十 TB 的数据。TuGraph 具有多种 API，使用户能够轻松构建应用程序，同时保持其应用程序的可优化性。

它有如下功能特征：

- 标签属性图模型
- 支持多图
- 完善的 ACID 事务处理
- 内置 25+ 图分析算法
- 基于 web 客户端的图可视化工具
- 支持 REST API 和 RPC
- OpenCypher 图查询语言
- 基于 C++/Python/Java 的存储过程
- 适用于高效图算法开发的 Traversal API

性能和可扩展性：

- TB 级大容量
- 千万顶点/秒的高吞吐率
- 高性能批量导入
- 在线/离线备份

## 数据模型

### 图模型

TuGraph 是一个具备多图能力的强模式属性图数据库。其支持最多一万亿顶点的有向图构建。

- 多图：在 TuGraph 中，每个数据库服务器可以承载多个图模型，每个图模型可以有自己的访问控制配置，数据库管理员可以创建或删除指定图模型。
- 属性图：TuGraph 中的顶点和边可以具有与其关联的属性，每个属性可以有不同的类型。
- 强模式：每个顶点和边必须有一个标签，且创建标签后，属性数量及类型较难被修改。
- 有向边：TuGraph 中的边为有向边，若要模拟无向边，用户可以创建两个方向相反的边。

### 数据类型

TuGraph 支持多种可用作属性的数据类型，具体支持的数据类型如下所示：

<caption>表 1. TuGraph 所支持的数据类型</caption>

| 数据类型 | 最小值              | 最大值              | 描述                                |
| -------- | ------------------- | ------------------- | ----------------------------------- |
| BOOL     | false               | true                | 布尔值                              |
| INT8     | -128                | 127                 | 8-bit 整型                          |
| INT16    | -32768              | 32767               | 16-bit 整型                         |
| INT32    | - 2^31              | 2^31 - 1            | 32-bit 整型                         |
| INT64    | - 2^63              | 2^63 - 1            | 64-bit 整型                         |
| DATE     | 0000-00-00          | 9999-12-31          | "YYYY-MM-DD" 格式的日期             |
| DATETIME | 0000-00-00 00:00:00 | 9999-12-31 23:59:59 | "YYYY-MM-DD hh:mm:ss"格式的时间日期 |
| FLOAT    |                     |                     | 32-bit 浮点数                       |
| DOUBLE   |                     |                     | 64-bit 浮点数                       |
| STRING   |                     |                     | 长度不定的字符串                    |
| BLOB     |                     |                     | 二进制数据                          |

_BLOB 类型的数据在输入输出时使用 BASE64 编码_

### 索引

TuGraph 支持对顶点字段进行索引。

索引可以是唯一索引或非唯一索引。如果为顶点标签创建了唯一索引，则 TuGraph 将在修改该标签的顶点时会先执行数据完整性检查，以确保该索引的唯一性。

每个索引都基于一个标签的一个字段构建，可以使用同一标签对多个字段进行索引。

BLOB 类型的字段不能建立索引。


# 安装

## 系统要求

表 2 是 TuGraph 对系统的配置要求。目前我们建议用户使用 NVMe SSD 配合较大的内存配置以获取最佳性能。

<caption>表2. TuGraph系统要求</caption>

|          | 最低配置  | 建议配置                 |
| -------- | --------- | ------------------------ |
| CPU      | X86_64    | Xeon E5 2670 v4          |
| 内存     | 4GB       | 256GB                    |
| 硬盘     | 100GB     | 1TB NVMe SSD             |
| 操作系统 | Linux 2.6 | Ubuntu 16.04, CentOS 7.3 |

## 通过 docker 镜像安装

安装 TuGraph 最简单的方法是通过 docker 镜像来安装和运行 TuGraph。同一个 docker 镜像可以在不同的 Linux 发行版本上运行而不需要再额外安装软件包，因此我们推荐用户使用这种方式进行安装。

想要使用 docker 镜像进行安装，用户首先需要确保自己的服务器中已经安装了 docker。以下命令可以判断 docker 是否已经安装：

```bash
$ sudo docker --version
```

如果该命令能顺利打印出 docker 版本号，则证明 docker 已经正常安装，否则需要先安装 docker。安装 docker 的过程可参考其官网 https://docs.docker.com/install/ 。

目前，TuGraph 提供基于 ubuntu 16.04 LTS 和 Centos 7.3 系统的镜像文件。用户可以联系我们以获取镜像：tugraph@service.alipay.com。

镜像文件是一个名为`lgraph_x.y.z.tar`的压缩文件，其中`x.y.z`是 TuGraph 的版本号。该压缩包可通过以下命令加载到 docker 镜像中：

```bash
$ sudo docker import tugraph_x.y.z.tar
```

如果加载成功，您的计算机上应具有名为 tugraph_x.y.z 的 docker 镜像，您可以使用以下命令运行该镜像：

```bash
$ sudo docker run -v /data_dir_on_host:/data_dir_in_docker -it tugraph_x.y.z /bin/bash
```

## Ubuntu 下的安装方法

除了通过 docker 镜像安装外，我们也提供了用于在 Ubuntu 上安装的 TuGraph 的.deb 安装包，其中包含了 TuGraph 可执行文件以及编写嵌入式程序和存储过程所需的头文件和相关库文件。

使用已经下载完成的`tugraph_x.y.z.deb`安装包在终端下安装，只需要运行以下命令：

```bash
$ sudo dpkg -i tugraph_x.y.z.deb
```

该命令默认将 TuGraph 安装于`/usr/local`目录下。用户也可以通过指定 `--instdir=<directory>` 选项更改安装目录。

## CentOS 下的安装方法

TuGraph 提供.rpm 安装包，包内所含功能模块与.deb 包一致。在 CentOS 系统下安装、卸载及使用过程也与 Ubuntu 系统下相似。
在终端下安装只需运行以下命令：

```bash
$ rpm -ivh tugraph-x.y.z.rpm
```

如果需要安装到指定目录，可以通过`--prefix`选项指定安装目录。

# 数据导入

在安装成功后，您可以使用`lgraph_import`批量导入工具将现有数据导入 TuGraph。

`lgraph_import`支持从 CSV 文件和 JSON 数据源导入数据。它有两种模式：

> jsonline 格式，一行一个 json 数组字符串

- _离线模式_：读取数据并将其导入指定服务器的数据文件，应仅在服务器离线时完成。
- _在线模式_：读取数据并将其发送到工作中的服务器，然后将数据导入其数据库。

## 离线全量导入

离线模式只能在离线状态的服务器使用。离线导入会创建一张新图，因此更适合新安装的 TuGraph 服务器上的第一次数据导入。

要在离线模式下使用`lgraph_import`工具，可以指定`lgraph_import --online false`选项。要了解可用的命令行选项，请使用`lgraph_import --online false --help`：

```bash
$ ./lgraph_import --online false -help
Available command line options:
    --log               Log file to use, empty means stderr. Default="".
    -v, --verbose       Verbose level to use, higher means more verbose.
                        Default=1.
    ...
    -h, --help          Print this help message. Default=0.
```

命令行参数如下：

- **-c, --config_file** `config_file`: 导入配置文件名，其格式要求见下述。
- **--log** `log_dir`: 日志目录。默认为空字符串，此时将日志信息输出到控制台。
- **--verbose** `0/1/2`: 日志等级，等级越高输出信息越详细。默认为 1。
- **-i, --continue_on_error** `true/false`: 在碰到错误时跳过错误并继续，默认为 false，碰到错误立即退出。
- **-d, --dir** `{diretory}`: 数据库目录，导入工具会将数据写到这个目录。默认为`./db`。
- **--delimiter** `{delimiter}`: 数据文件分隔符。只在数据源是 CSV 格式时使用，默认为`","`。
- **-u, --username** `{user}`: 数据库用户名。需要是管理员用户才能执行离线导入。
- **-p, --password** `{password}`: 指定的数据库用户的密码
- **--overwrite** `true/false`: 是否覆盖数据。设为 true 时，如果数据目录已经存在，则覆盖数据。默认为`false`。
- **-g, --graph** `{graph_name}`: 指定需要导入的图种类。
- **-h, --help**: 输出帮助信息。

CSV 格式的分隔符可以是单字符或多字符组成的字符串，其中不能包含`\r`或`\n`。注意不同的 shell 会对输入字符串做不同的处理，因此针对不同的 shell 输入参数可能需要不同的转义处理。
此外，`lgraph_import`还支持以下转义字符，以便输入特殊符号：

| 转义符 | 说明                                                           |
| ------ | -------------------------------------------------------------- |
| \\     | 反斜杠`\`                                                      |
| \a     | 响铃，即 ASCII 码 0x07                                         |
| \f     | form-feed，即 ASCII 码 0x0c                                    |
| \t     | 水平制表符，即 ASCII 码 0x09                                   |
| \v     | 垂直制表符，即 ASCII 码 0x0b                                   |
| \xnn   | 两位十六进制数，表示一个字节，如\x9A                           |
| \nnn   | 三位八进制数，表示一个字节，如\001, \443，数值范围不能超过 255 |

例：

```bash
$ ./lgraph_import -c ./import.config --delimiter "\001\002"
```

### 配置文件

`lgraph_import`工具通过指定的配置文件进行环境配置。配置文件描述输入文件的路径、它们所代表的顶点/边以及顶点/边的格式。

### 配置文件格式

配置文件有两大块组成`schema`和`files`。`schema`部分定义 label，`files`部分描述要导入的数据文件。

#### 所有的关键字

- schema (数组形式）
  - label（必选，字符串形式）
  - type（必选，值只能是 VERTEX 或者 EDGE）
  - properties（数组形式，对于点必选，对于边如果没有属性可以不配置）
    _ name（必选，字符串形式）
    _ type （必选，BOOL，INT8，INT16，INT32，INT64，DATE，DATETIME，FLOAT，DOUBLE，STRING，BLOB）
    _ optional（可选，代表该字段可以配置，也可以不配置）
    _ index（可选，该字段是否需要建索引）
    _ unique（可选，改字段是否建索引，并且是 unique 类型的）
    _ fulltext (可选，boolean 类型，该字段是否建全文索引，只能对 type 是 STRING 的字段设置))
  - primary (仅点配置，必选，主键字段，某一个 property，用来唯一确定一个点)
  - constraints (仅边配置，可选，数组形式，点 label 对，不配置或者为空代表不限制)
- files （数组形式）
  - path（必选，字符串，可以是文件路径或者目录的路径，如果是目录会导入此目录下的所有文件，需要保证有相同的 schema）
  - header（可选，数字，头信息占文件起始的几行，没有就是 0）
  - format（必须选，只能是 JSON 或者 CSV）
  - label（必选，字符串）
  - columns（数组形式）
    - SRC_ID (特殊字符串，仅边有，代表这列是起始点数据)
    - DST_ID (特殊字符串，仅边有，代表这列是目的点数据)
    - SKIP (特殊字符串，代表跳过这列数据)
    - [property]
  - SRC_ID (仅边配置，值是起始点标签)
  - DST_ID (仅边配置，值是目的点标签)

### 配置文件示例

```JSON
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
           "primary" : "aid"
        },
        {
            "label" : "movie",
            "type" : "VERTEX",
            "properties" : [
                {"name" : "mid", "type":"STRING"},
                {"name" : "name", "type":"STRING"},
                {"name" : "year", "type":"INT16"},
                {"name" : "rate", "type":"FLOAT", "optional":true}
            ],
           "primary" : "mid"
        },
        {
            "label" : "play_in",
            "type" : "EDGE",
            "properties" : [
                {"name" : "role", "type":"STRING", "optional":true}
            ],
            "constraints" : [["actor", "movie"]]
        }
    ],
    "files" : [
        {
            "path" : "actors.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "actor",
            "columns" : ["aid","name"]
        },
        {
            "path" : "movies.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "movie",
            "columns" : ["mid","name","year","rate"]
        },
        {
            "path" : "roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "SRC_ID" : "actor",
            "DST_ID" : "movie",
            "columns" : ["SRC_ID","role","DST_ID"]
        }
    ]
}
```

对于上述配置文件，定义了三个 label：两个点类型`actor`和`movie`，一个边类型`role`。每个 label 都描述了：label 的名字、类型（点还是边）、属性字段有哪些以及每个字段的类型。对于点，另外定义了 primary 字段是哪个；对于边，另外定义了 constraints 字段，用来限制边的起点和终点只能是哪些组合。

还描述了三个数据文件，两个点的数据文件`actors.csv`和`movies.csv`，一个边的数据文件`roles.csv`。每个部分都描述了：文件的路径（path）、数据类型（format）、信息头占开头几行（header）、是哪个 label 的数据（label）、文件中每行数据中的每个列对应的字段是哪个。

对于上述配置文件，import 工具在执行的过程中会先在 TuGraph 中创建`actor`、`movie`、`role`这三个 label，然后再执行三个文件的数据导入。

### 离线导入示例

在这个例子中，我们使用上面描述的电影-演员数据来演示导入工具的使用方法。待导入数据分为三个文件：`movies.csv`，`actors.csv`，以及`roles.csv`。

`movies.csv`包含的是电影的信息，其中每部电影有一个 id（作为检索的 primary key），此外每部电影还拥有 title、year 和 rating 等属性。（数据来自[IMDb](http://www.imdb.com)）。

```CSV
  [movies.csv]
  id, name, year, rating
  tt0188766,King of Comedy,1999,7.3
  tt0286112,Shaolin Soccer,2001,7.3
  tt4701660,The Mermaid,2016,6.3
```

> 对应的 jsonline 格式如下:
>
> ```JSON
> ["tt0188766","King of Comedy",1999,7.3]
> ["tt0286112","Shaolin Soccer",2001,7.3]
> ["tt4701660","The Mermaid",2016,6.3]
> ```
>
> 也可以所有字段都是字符串形式，导入的时候会转换成对应的类型
>
> ```JSON
> ["tt0188766","King of Comedy","1999","7.3"]
> ["tt0286112","Shaolin Soccer","2001","7.3"]
> ["tt4701660","The Mermaid","2016","6.3"]
> ```

`actors.csv`包含的是演员的信息。每个演员也拥有一个 id，以及 name 等属性。

```CSV
  [actors.csv]
  id, name
  nm015950,Stephen Chow
  nm0628806,Man-Tat Ng
  nm0156444,Cecilia Cheung
  nm2514879,Yuqi Zhang
```

> 对应的 jsonline 格式如下:
>
> ```JSON
> ["nm015950","Stephen Chow"]
> ["nm0628806","Man-Tat Ng"]
> ["nm0156444","Cecilia Cheung"]
> ["nm2514879","Yuqi Zhang"]
> ```

`roles.csv`则包含了演员在哪个电影中扮演了哪个角色的信息。其中每一行记录的是指定演员在指定电影里饰演的角色，对应数据库中的一条边。`SRC_ID` 和 `DST_ID` 分别是边的源顶点和目标顶点，他们分别是`actors.csv`和`movies.csv`中定义的`primary`属性。

```CSV
  [roles.csv]
  actor, role, movie
  nm015950,Tianchou Yin,tt0188766
  nm015950,Steel Leg,tt0286112
  nm0628806,,tt0188766
  nm0628806,coach,tt0286112
  nm0156444,PiaoPiao Liu,tt0188766
  nm2514879,Ruolan Li,tt4701660
```

> 对应的 jsonline 格式如下:
>
> ```JSON
> ["nm015950","Tianchou Yin","tt0188766"]
> ["nm015950","Steel Leg","tt0286112"]
> ["nm0628806",null,"tt0188766"]
> ["nm0628806","coach","tt0286112"]
> ["nm0156444","PiaoPiao Liu","tt0188766"]
> ["nm2514879","Ruolan Li","tt4701660"]
> ```

导入配置文件`import.conf`我们直接用上述的例子，来告诉`lgraph_import`工具如何导入这些文件：

注意每个文件中有两个标题行，因此我们需要指定`HEADER=2`选项。使用导入配置文件，我们现在可以使用以下命令导入数据：

```bash
$ ./lgraph_import
        -c import.conf             # 从import.conf读取配置信息
        --dir /data/lgraph_db      # 将数据存放在/data/lgraph_db
        --graph mygraph            # 导入名为 mygraph 的图
```

**注意**：

- 如果名为`mygraph`的图已存在，导入工具将打印错误消息并退出。要强制覆盖图形，可以使用`--overwrite true` 选项。
- 配置文件和数据文件必须使用 UTF-8 编码（或普通 ASCII 编码，即 UTF-8 的子集）存储。如果任何文件使用 UTF-8 以外的编码（例如，带有 BOM 或 GBK 的 UTF-8）编码，则导入将失败，并输出分析器错误。

## 在线增量导入

在线导入模式可用于将一批文件导入已在运行中的 TuGraph 实例中。这对于处理通常以固定的时间间隔进行的增量批处理更新非常便利。

`lgraph_import --online true`选项使导入工具能够在线模式工作。与`离线模式`一样，在线模式有自己的命令行选项集，可以使用`-h，--help`选项进行打印输出：

```bash
$ lgraph_import --online true -h
Available command line options:
    --online            Whether to import online.
    -h, --help          Print this help message. Default=0.

Available command line options:
    --log               Log file to use, empty means stderr. Default="".
    -v, --verbose       Verbose level to use, higher means more verbose.
                        Default=1.
    -c, --config_file   Config file path.
    -r, --url           DB REST API address.
    -u, --username      DB username.
    -p, --password      DB password.
    -i, --continue_on_error
                        When we hit a duplicate uid or missing uid, should we
                        continue or abort. Default=0.
    -g, --graph         The name of the graph to import into. Default=default.
    --skip_packages     How many packages should we skip. Default=0.
    --delimiter         Delimiter used in the CSV files
    --breakpoint_continue
                        When the transmission process is interrupted,whether
                        to re-transmit from zero package next time. Default=false
    -h, --help          Print this help message. Default=0.
```

文件的相关配置在配置文件中指定，其格式与`离线模式`完全相同。但是，我们现在不是将数据导入本地数据库，而是将数据发送到正在运行的 TuGraph 实例中，该实例通常运行在与运行导入工具的客户端计算机不同的计算机上。因此，我们需要指定远程计算机的 HTTP 地址的 URL、DB 用户和密码。

如果用户和密码有效，并且指定的图存在，导入工具将将数据发送到服务器，服务器随后解析数据并将其写入指定的图。数据将以大约 16MB 大小的包发送，在最近的换行符处中断。每个包都是以原子方式导入的，这意味着如果成功导入包，则成功导入所有数据，否则，任何数据都不会进入数据库。如果指定了`--continue_on_error true`，则忽略数据完整性错误，并忽略违规行。否则，导入将在第一个错误包处停止，并打印出已导入的包数。在这种情况下，用户可以修改数据以消除错误，然后使用`--skip_packages N`重做导入以跳过已导入的包。

# Client SDK

## Python 语言

#### client 构造函数，构造一个与 Tugraph Server 通信的客户端

    client(url,                     string type，Login address
           user,                    string type，The username.
           password)                string type，The password.

#### callCypher 执行一条 cypher 查询，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    callCypher(cypher,              string type，inquire statement.
               graph,               string type，the graph to query.
               json_format,         bool type，Returns format， true is json，Otherwise, binary format
               timeout)             double type，Maximum execution time, overruns will be interrupted

#### loadPlugin 加载一个用户自定义存储过程函数，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    loadPlugin(source_file,             string type，the source_file contain plugin code
                plugin_type,            string type，the plugin type, currently supported CPP and PY
                plugin_name,            string type，plugin name
                code_type,              string type，code type, currently supported PY, SO, CPP, ZIP
                plugin_description,     string type，plugin description
                read_only,              bool type，plugin is read only or not
                graph,                  string type，the graph to query.
                json_format,            bool type，Returns format， true is json，Otherwise, binary format
                timeout)                double type，Maximum execution time, overruns will be interrupted

#### callPlugin 执行一个已经加载的存储过程函数，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    callPlugin(plugin_type,         string type，the plugin type, currently supported CPP and PY
                plugin_name,        string type，plugin name
                param,              string type，the execution parameters
                plugin_time_out,    double type，maximum plugin execution time, overruns will be interrupted
                in_process,         support in future
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importSchemaFromFile 从文件中导入 schema，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    importSchemaFromFile(schema_file,         string type，the schema_file contain schema, the format is the same as lgraph_import
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importDataFromFile 从文件中导入点和边数据，此时应该已经导入了对应的 schema，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    importDataFromFile(conf_file,         string type，the plugin type, data file contain format description and data,the format is the same as lgraph_import
                delimiter,          string type，data separator
                continue_on_error,  string type，whether to continue when importing data fails
                thread_nums,        int type，maximum number of threads
                skip_packages,      int type，skip packages number
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importSchemaFromContent 从字节流中导入 schema，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    importSchemaFromContent(schema,         string type，the schema to imported， the format is the same as lgraph_import
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

#### importDataFromContent 从字节流导入点和边数据，此时应该已经导入了对应的 schema，返回(bool, string)类型元组，第一个值指示本次查询是否成功，成功时第二个值是查询结果，失败时为失败原因

    importDataFromContent(desc,        string type，the plugin type, data file contain format description and data, the format is the same as lgraph_import
                data,               string type，the data to be imported
                delimiter,          string type，data separator
                continue_on_error,  string type，whether to continue when importing data fails
                thread_nums,        int type，maximum number of threads
                skip_packages,      int type，skip packages number
                graph,              string type，the graph to query.
                json_format,        bool type，Returns format， true is json，Otherwise, binary format
                timeout)            double type，Maximum execution time, overruns will be interrupted

# 服务器配置

## 配置参数

TuGraph 服务器在启动时从配置文件和命令行选项加载配置，如果在配置文件和命令行中同一选项指定了不同的值，将优先使用命令行中指定的值。

具体参数及其类型描述如下：

| 参数名                           | <nobr>参数类型</nobr> | 参数说明                                                                                |
|:------------------------------| --------------------- |:------------------------------------------------------------------------------------|
| directory                     | 字符串                | 数据文件所在目录。如果目录不存在 ，则自动创建。默认目录为 `/var/lib/lgraph/data`。                               |
| durable                       | 布尔值                | 是否开启实时持久化。关闭持久化可以减少写入时的磁盘 IO 开销，但是在机器断电等极端情况下可能丢失数据。默认值为 `true`。                    |
| host                          | 字符串                | REST 服务器监听时使用的地址，一般为服务器的 IP 地址。默认地址为 `0.0.0.0`。                                     |
| port                          | 整型                  | REST 服务器监听时使用的端口。默认端口为 `7070`。                                                      |
| enable_rpc                    | 布尔值                | 是否使用 RPC 服务。默认值为 0。                                                                 |
| rpc_port                      | 整型                  | RPC 及 HA 服务所用端口。默认端口为 `9090`。                                                       |
| ha_log_dir                    | 字符串                | HA 日志所在目录，需要启动 HA 模式。默认值为空。                                                         |
| master                        | 字符串                | 根据 host1:port1,host2:port2 初始化节点。默认值为空。                                             |
| verbose                       | 整型                  | 日志输出信息的详细程度。可设为 `0`，`1`，`2`，值越大则输出信息越详细。默认值为 `1`。                                   |
| log_dir                       | 字符串                | 日志文件所在的目录。默认目录为 `/var/log/lgraph/`。                                                 |
| ssl_auth                      | 布尔值                | 是否使用 SSL 安全认证。当开启时，REST 服务器只开启 HTTPS 服务。默认值为 `false`。                               |
| web                           | 字符串                | web 文件（包含可视化部分）所在目录。默认目录为 `/usr/local/share/lgraph/resource`。                       |
| server_cert                   | 字符串                | 在 SSL 认证开启时，服务器所使用的 certificate 文件路径。默认路径为 `/usr/local/etc/lgraph/server-cert.pem`。 |
| server_key                    | 字符串                | 在 SSL 认证开启时，服务器所使用的公钥文件。默认目录为 `/usr/local/etc/lgraph/server-key.pem`。               |
| enable_audit_log              | 布尔值                | 是否启用审计日志，默认值为 `false`。                                                              |
| audit_log_expire              | 整型                  | 启用审计日志时，日志的有效时间（小时），超时自动清理，值为 0 时表示不清理。默认值为 `0`。                                    |
| audit_log_dir                 | 字符串                | 启用审计日志时，日志文件的存放目录。默认目录为 `$directory/_audit_log_`。                                   |
| load_plugins                  | 布尔值                | 启动服务时导入所有存储过程。默认值为 1。                                                               |
| optimistic_txn                | 布尔值                | 为 Cypher 开启乐观多线程写入事务。默认为 0。                                                         |
| disable_auth                  | 布尔值                | 关闭 REST 验证。默认为 0。                                                                   |
| snapshot_interval             | 整型                  | 快照间隔（以秒为单位）。 默认值为 86400。                                                            |
| heartbeat_interval_ms         | 整型                  | 心跳间隔（以毫秒为单位）。 默认值为 1000。                                                            |
| heartbeat_failure_duration_ms | 整型                  | 心跳超时且节点下线间隔（以毫秒为单位）。默认为 60000。                                                      |
| node_dead_duration_ms         | 整型                  | 节点被视为完全死亡并从列表中删除的间隔（以毫秒为单位）。默认值为 120000。                                            |
| enable_ip_check               | 布尔值                | 允许 IP 白名单，默认值为 0。                                                                   |
| idle_seconds                  | 整型                  | 子进程可以处于空闲状态的最大秒数。 默认值为 600。                                                         |
| enable_backup_log             | 布尔值                | 是否启用备份日志记录。 默认值为 0。                                                                 |
| backup_log_dir                | 字符串                | 存储备份文件的目录。 默认值为空。                                                                   |
| snapshot_dir                  | 字符串                | 存储快照文件的目录。 默认值为空。                                                                   |
| thread_limit                  | 整型                  | 同时使用的最大线程数。 默认值为 0，即不做限制。                                              |
| enable_fulltext_index         | 布尔值                | 是否启用全文索引功能，默认值为 0。                                                                  |
| fulltext_analyzer             | 字符串                | 全文索引分词器类型。可设为`StandardAnalyzer`或者`SmartChineseAnalyzer`。默认是`StandardAnalyzer`       |
| fulltext_commit_interval      | 整形                  | 全文索引数据提交周期,针对写操作，单位秒。默认是 0，立即提交。                                                    |
| fulltext_refresh_interval     | 整形                  | 全文索引数据刷新周期，针对读操作，单位秒。默认是 0，立即可以读到最新写入的数据。                                           |
| help                          | 布尔值                | 打印此帮助消息。 默认值为 0。                                                                    |

## 服务器配置文件

TuGraph 的配置文件以 JSON 格式存储。建议将大多数配置存储在配置文件中，并且仅在需要时使用命令行选项临时修改某些配置参数。

一个典型的配置文件如下：

```json
{
  "directory": "/var/lib/lgraph/data",

  "port": 7090,
  "rpc_port": 9090,
  "enable_ha": false,

  "verbose": 1,
  "log_dir": "/var/log/lgraph/",

  "ssl_auth": false,
  "server_key": "/usr/local/etc/lgraph/server-key.pem",
  "server_cert": "/usr/local/etc/lgraph/server-cert.pem"
}
```

## 命令行参数

`lgraph_server`命令用于启动 TuGraph 服务器实例。
命令行选项可用于在启动 TuGraph 服务器时覆盖从配置文件加载的配置。

除了 [配置参数](#配置参数)中描述的所有配置选项外，`lgraph_server`也会确定已启动服务器的运行模式，运行模式可以为`standard`或`daemon`，有关运行模式的具体细节可参阅 [运行模式](#运行模式)。

一个典型的`lgraph_server`命令如下：

```bash
$ lgraph_server --config ./local_lgraph.json --port 7777 --mode start
```

# 服务启停

## 运行模式

TuGraph 可以作为前台普通进程启动，也可以作为后台守护进程启动。

当作为普通进程运行时，TuGraph 可以直接将日志打印到终端，这在调试服务器配置时非常方便。但是，由于前台进程在终端退出后被终止，因此用户须确保在 TuGraph 服务器处于运行状态时，终端保持打开状态。

另一方面，在守护进程模式下，即使启动它的终端退出，TuGraph 服务器也可以继续运行。因此，在长时间运行的服务器下推荐以守护进程模式启动 TuGraph 服务器。

## 运行普通进程

`lgraph_server -d run`命令可以将 TuGraph 作为普通进程运行。普通进程依赖命令行终端，因此终端结束时，TuGraph 进程也会自动终止。普通进程模式配合`--log_dir ""`可以将进程日志直接输出到终端，因此更方便调试。

普通模式的运行输出示例如下所示：

```bash
$ ./lgraph_server -c lgraph_standalone.json --log_dir ""
20200508120723.039: **********************************************************************
20200508120723.039: *                  TuGraph Graph Database v3.0.0                     *
20200508120723.040: *                                                                    *
20200508120723.041: *        Copyright(C) 2018 Ant Group. All rights reserved.           *
20200508120723.041: *                                                                    *
20200508120723.044: *             Licensed host: hostname      threads:0, ha:0           *
20200508120723.044: **********************************************************************
20200508120723.044: Server is configured with the following parameters:
20200508120723.045:   data directory:    ./lgraph_db
20200508120723.046:   enable ha:          0
20200508120723.046:   durable:            1
20200508120723.047:   host:               127.0.0.1
20200508120723.047:   REST port:          7071
20200508120723.048:   RPC port:           9091
20200508120723.048:   enable rpc:         0
20200508120723.051:   optimistic txn:     0
20200508120723.059:   verbose:            1
20200508120723.074:   log_dir:
20200508120723.074:   ssl_auth:           0
20200508120723.075:   resource dir:       ./resource

20200508120723.077: Loading DB state from disk
20200508120723.110: [RestServer] Listening for REST on port 7071
20200508120723.110: [LGraphService] Server started.
```

普通进程模式下，用户可以通过按 CTRL+C 来提前终止 TuGraph 进程。

## 启动服务

TuGraph 需要通过 `lgraph_server -d start` 命令行启动，启动命令示例如下：

```bash
$ ./lgraph_server -d start -c lgraph_daemon.json
Starting lgraph...
The service process is started at pid 12109.
```

此命令启动的 TuGraph 服务器进程为守护进程，它将从文件`lgraph_daemon.json`加载相关配置。

服务器启动后，它将开始在日志文件中打印日志，之后可用该日志文件确定服务器的状态。

## 停止服务

用户可以使用`kill`命令以及`lgraph_server -d stop`命令停止 TuGraph 守护进程。

由于可能在同一台计算机上运行多个 TuGraph 服务器进程，因此我们使用`.pid`文件区分不同的服务器进程，该文件写入启动该进程的工作目录。因此，需要在相同工作目录中运行`lgraph_server-d stop`命令，以停止正确的服务器进程。

```bash
user@host:~/tugraph$ ./lgraph_server -d start -c lgraph_standalone.json
20200508122306.378: Starting lgraph...
20200508122306.379: The service process is started at pid 93.

user@host:~/tugraph$ cat ./lgraph.pid
93

user@host:~/tugraph$ ./lgraph_server -d stop -c lgraph_standalone.json
20200508122334.857: Stopping lgraph...
20200508122334.857: Process stopped.
```

## 重启服务

用户也可以通过`lgraph_server -d restart`来重启 TuGraph 服务：

```bash
$ ./lgraph_server -d restart
Stopping lgraph...
Process stopped.
Starting lgraph...
The service process is started at pid 20899.
```

# 使用 API 访问数据库

在 TuGraph 数据库中访问数据也有多种方法：

- **REST API:** TuGraph 提供了一组涵盖数据库的基本操作的 REST API，包括调用 Cypher 查询和调用插件。REST API 的完整文档请参阅 [TuGraph REST API Manual](./TuGraph-Rest-API.md)。
- **OpenCypher 查询语言:** TuGraph 支持开源图数据库查询语言 OpenCypher。除了 OpenCypher 的标准语法之外，TuGraph 还有自己的扩展。有关如何在 TuGraph 中使用扩展 OpenCypher 的详细信息，请参阅 [TuGraph OpenCypher Manual](/TuGraph-Cypher.md)。
- **插件 API:** 除了 OpenCypher 查询之外，TuGraph 还提供对用户公开低优先级操作的插件 API。用户可以使用插件 API 编写插件并将其加载到服务器中。由于插件是使用命令性语言编写的（目前我们支持 C++、Java 和 Python），因此它们可用于实现任意复杂的逻辑。如果考虑到性能问题，插件可以用原生语言编写以方便优化到最佳。插件 API 以及如何管理插件的参考请参阅 [TuGraph Plugin Manual](./TuGraph-Procedure.md)。

用户可以通过以下方式访问 TuGraph 服务器：

- **`lgraph_cypher` 命令行查询客户端**: 在服务器上执行 OpenCypher 查询并在终端中打印结果的命令行工具。
- **[TuGraph 可视化工具](#可视化工具):** 可用于发送 Cypher 查询和可视化生成的子图的 Web 界面，它也可以用来管理和调用插件。
- **HTTP 请求:** TuGraph 直接使用 HTTP 提供 REST 请求，用户可以使用 HTTP 客户端将 REST 请求发送到 TuGraph 服务器。
- **TuGraph 客户端 SDK:** TuGraph 以多种语言（当前 C++、Java 和 Python）提供客户端 SDK，以供用户应用程序调用。

`lgraph_cypher`和 TuGraph 可视化工具提供交互式界面，因此更适合人工使用，而 REST API 和 SDK 则设计为程序使用。

# `lgraph_cypher` 使用说明

TuGraph 发布版本附带名为`lgraph_cypher`的查询客户端，可用于向 TuGraph 服务器提交 OpenCypher 请求。`lgraph_cypher`客户端有两种执行模式：单命令模式和交互式模式。

## 单命令模式

在单命令模式下，`lgraph_cypher`可用于提交单个 Cypher 查询并将结果直接打印到终端，打印结果也可以容易地重定向写入指定文件。当用户需要从服务器获取大量结果并将其保存在文件中时，这非常便利。

在此模式下，`lgraph_cypher`工具具有以下选项：

### 命令行参数:

| 参数     | 类型   | 说明                                                                                                                                      |
| -------- | ------ | ----------------------------------------------------------------------------------------------------------------------------------------- |
| --help   | \      | 列出所有参数及说明。                                                                                                                      |
| -example | \      | 列出命令实例。                                                                                                                            |
| -c       | string | 数据库的配置文件，用于获取 ip 与 port 信息。                                                                                              |
| -h       | string | 数据库服务器 ip 地址，如有配置文件则可舍去此参数。默认值为`127.0.0.1`。                                                                   |
| -p       | string | 数据库服务器端口，如有配置文件则可舍去此参数。默认值为`7071`。                                                                            |
| -u       | string | 数据库登录用户名。                                                                                                                        |
| -P       | string | 数据库登录密码。                                                                                                                          |
| -f       | string | 包含单条 Cypher 查询单文本文件的路径。                                                                                                    |
| -s       | string | 单行 cypher 查询命令。以`"`开头结尾。                                                                                                     |
| -t       | int    | 进行 cypher 查询时服务器的超时阈值。默认值为`150`秒。                                                                                     |
| -format  | string | 查询结果显示模式。支持`plain`与`table`两种格式。`plain`格式会将查询结果单列打印。`table`格式会将查询结果以表格方式显示。默认值为`table`。 |

### 命令示例:

**cypher 命令文件查询：**

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -f /home/usr/cypher.json
```

**cypher 命令单句查询：**

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u user -P password -s "MATCH (n) RETURN n"
```

## 交互模式

`lgraph_cypher`也可以在交互模式下运行。在交互式模式下，客户端与服务器保持连接，并在读取-评估-打印-循环中与用户进行交互。

### 进入 lgraph_cypher 交互模式:

如不加`-f`或`-s`命令行选项，运行`lgraph_cypher`时将会进入交互模式。使用方式如下：

```
$ ./lgraph_cypher.py -c /home/usr/lgraph_standalone.json -u admin -P 73@TuGraph
```

如成功进入则会显示相应登录成功信息：

```
**********************************************************************
*                  TuGraph Graph Database X.Y.Z                      *
*                                                                    *
*        Copyright(C) 2018 Ant Group. All rights reserved.           *
*                                                                    *
**********************************************************************
login success
----------------------------------
Host: 127.0.0.1
Port: 7071
Username: admin
----------------------------------
type ":help" to see all commands.
>
```

现在我们也提供一个交互式 shell ，用于用户输入 Cypher 查询语句或使用`:help`命令来检查可用命令。

### command 种类与说明:

除 Cypher 查询外，`lgraph_cypher` 的 shell 还接受以下命令：

| 命令                     | 对应参数                           | 说明                                                                                                  |
| ------------------------ | ---------------------------------- | ----------------------------------------------------------------------------------------------------- |
| :help                    | \                                  | 显示服务器信息与所有 command 对应说明。                                                               |
| :db_info                 | \                                  | 当前服务器状态查询。对应 REST API 的/db/info。                                                        |
| :clear                   | \                                  | 清空屏幕。                                                                                            |
| :use                     | {图的名称}                         | 使用该名称指定的图，默认值为`default`。                                                               |
| :source                  | `-t {查询timeout值} -f {查询文件}` | 可交互模式下的 cypher 命令文件查询。超时阈值默认值为`150`秒。查询文件格式参考无交互式查询参数。       |
| :exit                    | \                                  | 退出交互模式并返回原命令行。                                                                          |
| :format                  | `plain` or `table`                 | 更改 cypher 查询结果的显示模式。支持`plain`与`table`模式。                                            |
| :save all/command/result | `-f {文件路径}` `{cypher语句}`     | 存储 cypher 命令（command）或查询结果（result）或以上二者（all）。默认存储位置为`/saved_cypher.txt`。 |

**注意:**

- 每条命令都应该以冒号开始 `:`.

**:save 命令例子:**

```
:save all -f /home/usr/saved.txt match (n) where return n, n.name limit 1000
```

### cypher 查询命令:

在交互模式下，用户也可直接输入单句 cypher 命令进行查询，以"`;`"结束。输入命令不区分大小写。例子如下：

```
login success
>MATCH (n) RETURN n, n.name;
+---+---+-------------+
|   | n |n.name       |
+---+---+-------------+
| 0 | 0 |david        |
| 1 | 1 |Ann          |
| 2 | 2 |first movie  |
| 3 | 3 |Andres       |
+---+---+-------------+
time spent: 0.000520706176758
size of query: 4
>
```

`lgraph_cypher`输入命令时支持多行输入，用户可使用`ENTER`键将长查询语句分多行输入。多行输入情况下命令行开头会从`>`变为`=>`，然后用户可以继续输入查询的其余部分。

例子如下：

```
login success
>MATCH (n)
=>WHERE n.uid='M11'
=>RETURN n, n.name;
```

### 辅助功能:

**历史查询：** 在交互模式下按上下方向键可查询输入历史。

**自动补全：** lgraph_cypher 会根据输入历史进行自动补全。在补全提示出现的情况下，按下右方向键就会自动补全命令。

# 可视化工具

TuGraph 提供基于 Web 的可视化界面，使用户能够：

- 执行 Cypher 查询并对生成的子图进行可视化
- 管理数据库中的图
- 管理和调用插件
- 实时检查数据库状态
- 管理用户帐户和对单个图的访问权限
- 管理数据库中当前运行的任务
- 分析审计日志

有关 TuGraph 可视化工具的更详细说明，请参阅 [TuGraph Visualizer Manual](./TuGraph-Visualizer.md)。

# 图神经网络

## 简介

TuGraph 实现了 DGL 库的集成，DGL 是一个开源的高性能图神经网络框架，能兼容 PyTorch、Apache MXNet 以及 TensorFlow 等主流机器学习库，并提供了多种常见的图学习算法。
DGL 文档可参考 [https://docs.dgl.ai/index.html](https://docs.dgl.ai/index.html) 。

主要功能包括：

- 图数据从 TuGraph 文件到 DGLGraph 结构的转换
- 集成了基于 PyTorch 开发的各图学习算法
- 结果写回到 TuGraph 中（开发中）

## 环境配置

- 要求 Python 版本 3.6 及以上
- 安装 pytorch
- 通过 pip 或 conda 安装 DGL，安装命令可参考[https://www.dgl.ai/pages/start.html](https://www.dgl.ai/pages/start.html)

## 调用过程

### 图数据接入

通过 galaxy 实例构建图数据遍历入口：

```python
db_path = "db_cora"
username = "admin"
password = "73@TuGraph"
graph = "default"

galaxy = Galaxy(args.db_path)
galaxy.SetCurrentUser(username, password)
graphDB = galaxy.OpenGraph(graph, False)
txn = graphDB.CreateReadTxn()
```

### 图数据遍历

遍历所有点出边，构建 src 数组和 dst 数组。DGL 构建图需要将边数据拆分为起始点 src 数组和终点 dst 数组，要求两数组大小一致。

```python
vit = txn.GetVertexIterator(0)

while(True):
    nbr_list = vit.ListDstVids()
    vid = vit.GetId()
    labels.append(vit.GetField("label"))
    for nbr in nbr_list[0]:
        src.append(vid)
        dst.append(nbr)
    if (not vit.Next()):
        break

labels = torch.tensor(labels, dtype=torch.int64)
src, dst = torch.tensor(src, dtype=torch.int32), torch.tensor(dst, dtype=torch.int32)
g = dgl.graph((src, dst))
g.ndata['label'] = labels
```

实例代码分为两步：

- 通过 VertexIterator 指针遍历所有顶点，并将所有出边拆分后添加到 src 和 dst 数组。其中 labels 为点属性数组
- 通过 src 和 dst 构建 dgl.graph，并将 labels 属性添加到 dgl.graph 中

### 关闭 TuGraph 数据入口

在非**main**函数中通过 graphDB 和 txn 遍历图数据后，必须关闭 TuGraph 数据入口，否则会出现未知错误，目前仍在调试问题中。

```python
txn.Commit()
graphDB.Close()
galaxy.Close()
```

### 算法调用

dgl 所有算法均通过 dgl.graph 进行数据导入，通过上述两步构建 dgl.graph 并提取点、边属性后， 可集成并调用各算法。

```python
def compute_pagerank(g):
    g.ndata['pv'] = torch.ones(N) / N
    degrees = g.out_degrees(g.nodes()).type(torch.float32)
    for k in range(K):
        g.ndata['pv'] = g.ndata['pv'] / degrees
        g.update_all(message_func=fn.copy_src(src='pv', out='m'),
                     reduce_func=fn.sum(msg='m', out='pv'))
        g.ndata['pv'] = (1 - DAMP) / N + DAMP * g.ndata['pv']
    return g.ndata['pv']
```

# 全文索引

TuGraph 支持对点或者边的字段建立全文索引。

全文索引的实现基于开源的 Lucene 引擎，由于全文索引的数据和点边的数据不在一个事务里面，因此全文索引不保证事务一致性。

只能对`STRING`类型的字段设置全文索引。

## 添加全文索引

首先需要服务器配置参数添加 `enable_fulltext_index = true`，开启全文索引功能。

对于离线全量导入，需要在`lgraph_import`导入工具的参数中添加`--enable_fulltext_index true`和`--fulltext_analyzer StandardAnalyzer`

### 创建点或边的同时设置全文索引

在点或者边的 schema 定义中，对某个字段添加`"fulltext" : true`属性即可。如下对`actor`和`play_in`的`name`属性设置全文索引。

```JSON
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING", "fulltext": true}
            ],
           "primary" : "aid"
        },
        {
            "label" : "play_in",
            "type" : "EDGE",
            "properties" : [
                {"name" : "role", "type":"STRING", "optional":true, "fulltext": true}
            ],
            "constraints" : [["actor", "movie"]]
        }
    ]
}
```

之后在点和边的插入、修改、删除操作中会自动同步全文索引数据。

## 对已经存在的点或者边设置或者删除全文索引

此时需要通过`procedure`进行，分两步进行：先更改索引属性（轻量级操作），再重建索引（耗时操作）。

```shell
#对点actor的name属性设置全文索引
CALL db.addFullTextIndex(true, "actor", "name");
#对点actor的name属性删除全文索引
CALL db.deleteFullTextIndex(true, "actor", "name");
#对边play_in的role属性设置全文索引
CALL db.addFullTextIndex(false, "play_in", "role");
#对边play_in的role属性删除全文索引
CALL db.deleteFullTextIndex(false, "play_in", "role");
```

如上操作完以后，执行下面的重建全文索引操作。

```shell
# 重建点actor和边play_in的全文索引数据
CALL db.rebuildFullTextIndex('["actor"]', '["play_in"]');
```

如果发现全文索引数据不一致，可选择调用`CALL db.rebuildFullTextIndex`重建索引。

## 全文索引参数

全文索引默认不开启，打开后默认设置是：写入是立即提交，读取是立即可读，这种情况数据的实时性较高，但写入性能较差。

另外也可以设置异步周期性提交和异步周期性刷新数据，这种情况数据不是实时可见的，但会提高写入性能。

全文索引的`commit`和`refresh`是两个独立的操作，`commit`成功仅代表异常重启数据不会丢失了，`refresh`成功仅代表可以读到最新写入的数据，包括还未提交的。

```shell
# 打开全文索引功能。
enable_fulltext_index = true;
# StandardAnalyzer英文分词效果好，中文分词可设置为SmartChineseAnalyzer
fulltext_analyzer = "StandardAnalyzer";
#周期性60秒commit一次数据，类似于批量写入。如果设置为0，立即commit。
fulltext_commit_interval = 60;
#周期性5秒refresh一次数据，即最新写入的数据5秒钟之后才可以被读到。如果设置为0，立即可读。
fulltext_refresh_interval = 5;
```

# 高可用模式

TuGraph 通过多机热备份来提供高可用模式（HA 模式）。在高可用模式下，对数据库的写操作会被同步到所有服务器上，这样即使有部分服务器宕机也不会影响服务的可用性。
该功能将在Enterprise版本的TuGraph中提供支持。

# 数据库管理

## 日志信息

TuGraph 保留两种类型的日志：服务器日志和审计日志。服务器日志记录人为可读的服务器状态信息，而审核日志维护服务器上执行的每个操作加密后的信息。

### 服务器日志

服务器日志会跟踪服务器的状态信息（如服务器启动和停止等）以及服务器已提供的请求及其相应的响应。服务器日志的详细程度可通过`verbose`选项进行配置。日志的位置在`log_dir`选项中指定。

默认的`verbose`等级为`1`，此等级下，服务器将仅打印主要事件的日志，如服务器启动/停止。请求和响应不会记录在此级别。

### 审计日志

审核日志记录每个请求和响应，以及发送请求的用户以及收到请求的时间。审核日志只能是打开或关闭状态。可以使用 TuGraph 可视化工具和 REST API 查询结果。

## 数据导出

TuGraph 可以通过 `lgraph_export` 工具来对已经导入成功的数据库进行数据导出。 `lgraph_export` 工具可以将指定 TuGraph 数据库的数据以 `csv` 或者 `json` 文件形式导出到指定目录，同时导出这些数据进行再导入时需要的配置文件 `import.config` ，详细描述可参见[配置文件](#配置文件)。

该工具的命令示例如下：

```bash
$ lgraph_export -d {database_dir} -e {export_destination_dir} -g {graph_to_use} -u {username} -p {password} -f {output_format}
```

其中：

- `-d {database_dir}` 指定需要进行数据导出的数据库所在目录，默认值为 `./testdb`。
- `-e {export_destination_dir}` 指定导出文件存放的目录，默认值为 `./exportdir`。
- `-g {graph_to_use}` 指定图数据库的种类，默认为 `default` 。
- `-u {username}` 指定进行该导出操作的用户的用户名。
- `-p {password}` 指定进行该导出操作的用户的用户密码。
- `-s {field_separator}` 指定导出文件的分隔符，默认为逗号。
- `-f {output_format}` 指定导出数据的格式，`json`或者`csv`，默认为`csv`。
- `-h` 除上述指定参数外，也可以使用该参数查看该工具的使用帮助。

## 数据备份

TuGraph 可以通过 `lgraph_backup` 工具来进行数据备份。
`lgraph_backup` 工具可以将一个 TuGraph 数据库中的数据备份到另一个目录下，它的用法如下：

```bash
$ lgraph_backup -s {source_dir} -d {destination_dir} -c {true/false}
```

其中：

- `-s {source_dir}` 指定需要备份的数据库（源数据库）所在目录。
- `-d {destination_dir}` 指定备份文件（目标数据库）所在目录。
  如果目标数据库不为空，`lgraph_backup` 会提示是否覆盖该数据库。
- `-c {true/false}` 指明是否在备份过程中进行 compaction。
  compaction 能使产生的备份文件更紧凑，但备份时间也会变长。该选项默认为 `true`。

## 数据预热

TuGraph 是基于磁盘的数据库，仅当访问数据时，数据才会加载到内存中。因此在服务器刚开启后的一段时间内，系统性能可能会由于频繁的 IO 操作而变差。此时我们可以通过事先进行数据预热来改善这一问题。

数据预热可以通过工具 `lgraph_warmup` 来进行。它的使用示例如下：

```bash
$ lgraph_warmup -d {directory} -g {graph_list}
```

其中：

- `-d {db_dir}` 选项指定了 TuGraph 服务器的数据目录

* `-g {graph_list}` 选项指定需要进行数据预热的图名称，用逗号分隔

根据数据大小和所使用的磁盘类型不同，预热过程运行时间也不同。机械磁盘上预热一个大数据库可能耗时较长，请耐心等待。

## 任务管理

TuGraph 会跟踪长时间运行的任务。可以使用 REST API 、Cypher 以及 TuGraph 可视化工具查询当前正在运行的任务列表。长时间运行的任务可以由数据库管理员终止。

## 内存资源隔离

TuGraph 可为每个用户设置内存资源的限制，当某个用户执行语句超过内存限制时，会终止执行并返回错误信息。通过 cypher 语句`CALL dbms.setUserMemoryLimit`来进行设置，未设置则没有限制。用户也可以通过 cypher 语句获取用户的内存实时用量。

目前内存资源仅对 Cypher 语句中的核心部分插装监控，不包括 plugin 中的内存申请和释放。

# 资源监控

lgraph_monitor可以监控Tugraph所在机器的资源使用情况与服务的请求统计情况,它会定期的发送RPC请求到Tugraph查询当前状态，并将查询结果导入到prometheus中,前端可以使用grafana查询prometheus中的时序数据来展示机器和服务状态。

## 启动tugraph
```bash
./lgraph_server -c lgraph_standalone.json
```

##启动prometheus
1. 安装prometheus，详见 https://prometheus.io/
2. 配置prometheus.yml文件监听lgraph_monitor监控系统
3. 启动prometheus

## 监控启动

```bash
$ ./lgraph_monitor -u admin -p 73@TuGraph --monitor_host 127.0.0.1:9999 --sampling_interval_ms 150
Available command line options:
    --server_host           Host on which the tugraph rpc server runs. Default="127.0.0.1:9091"
    -u, --user              DB username
    -p, --password          DB password
    --monitor_host          Host on which the monitor restful server runs. Default="127.0.0.1:9999".
    --sampling_interval_ms  sampling interval in millisecond. Default=150
```

## 安装 grafana 模板

1. 安装并启动 grafana，详见 https://grafana.com/
2. 配置 grafana 数据源，选择 prometheus 数据源，并填写 IP 地址和端口
3. 点击“+”，选择 import，选择”Upload JSON file
   “，选择 grafana 模板文件（/deps/TuGraph-web/grafana-template/TuGraph-grafana-template.json）

# 常见问题

**1. "Error opening config file xxx, exiting..."**

这一错误出现在启动 TuGraph 服务时，原因是程序无法正确读取该配置文件。请确认该配置文件路径正确，并且具有读取权限。

**2. "Failed to parse command line option: Option xxx cannot be recognized."**

这一错误出现在使用 TuGraph 的命令行时，原因是该命令行参数无法正确识别，一般是因为参数名错误。可以使用`-h, --help`来打印符合要求的选项。


