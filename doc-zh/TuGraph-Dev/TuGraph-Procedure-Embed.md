# TuGraph Stored Procedure Embed Mode Guide

Version: 3.3.0

2022/03/29

蚂蚁集团

---

# 1. 简介

内容仅针对开发人员。Embed模式为TuGraph存储过程的调试方式。

常规的存储过程的使用方式是对编译后的存储过程文件(.so/.py)安装、调用，依赖TuGraph Server。Embed模式仅依赖TuGraph的数据文件。

存储过程的实现方式参考`doc/TuGraph-Procedure/TuGraph-Procedure.md`。

# 2. C++存储过程Embed模式

## 2.1 依赖

- 入口文件：embed_main.cxx
  - 入口文件包含了：数据路径、图名、账号、密码、存储过程参数

- 存储过程文件：*.cpp

### 2.1.1 入口文件示例：

参考：`./plugins/embed_main.cxx`

```cpp
#include <iostream>
#include "lgraph/lgraph.h"
using namespace std;

extern "C" bool Process(lgraph_api::GraphDB &db,
                        const std::string &request,
                        std::string &response);

int main(int argc, char** argv) {
    lgraph_api::Galaxy g("lgraph_db");
    g.SetCurrentUser("admin","73@TuGraph");
    lgraph_api::GraphDB db = g.OpenGraph("default");
    std::string resp;
    bool r = Process(db, "{\"scan_edges\":true, \"times\":1}", resp);
    cout << r << endl;
    cout << resp << endl;
    return 0;
}
```

## 2.2 编译

编译脚本参考：`./plugins/make_embed.sh`

以编译pagerank.cpp为例（LG_INCLUDE, LG_LIB需要根据部署路径修改）：

```bash
PLUGIN_CPP=pagerank.cpp
LG_INCLUDE=/usr/local/include
LG_LIB=/usr/local/lib64
g++ -fno-gnu-unique -fPIC -g --std=c++14 -I${LG_INCLUDE} -rdynamic -O3 -fopenmp -DNDEBUG -o embed ${PLUGIN_CPP} embed_main.cxx "${LG_LIB}/liblgraph.so" -lrt
```

## 2.3 执行

```bash
./embed
```

# 3. Python存储过程Emded模式

## 3.1 依赖

- 入口文件：
  - 入口文件包含了：数据路径、图名、账号、密码、存储过程参数、Python存储过程文件名
- 存储过程文件：*.py
- 动态库：lgraph_python.so

### 3.1.1 入口文件示例：

参考：`./plugins/embed_main.py`

以scan_graph为例

```python
from lgraph_python import GraphDB, Galaxy
import json

# --- add python plugin ---
import python.scan_graph as python_plugin
# --- add python plugin ---

if __name__ == "__main__":
    galaxy = Galaxy("lgraph_db")
    galaxy.SetCurrentUser("admin", "73@TuGraph")
    db = galaxy.OpenGraph("default", False)
    res = python_plugin.Process(db, "{\"scan_edges\": true}")
    print(res)
    db.Close()
    galaxy.Close()
```

## 3.2 执行

```bash
python3 embed_main.py
```

