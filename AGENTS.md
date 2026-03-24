# Repository Guidelines
## 项目背景
- 这是个图数据库项目，C++编译，使用OpenCypher查询语言，功能对标Neo4j。
- 项目还没有投入生产，仍然处于开发中，**更改代码不用考虑兼容性，不要有任何兼容性代码**。

## 项目结构与模块划分
- `src/` 是核心代码目录。
- `src/graphdb/` 负责存储与索引，`src/cypher/` 负责查询解析与执行。
- `src/server/` 提供 `lgraph_server` 入口，`src/bolt/` 处理 Bolt 协议。
- `src/transaction/`、`src/common/`、`src/toolkits/` 分别对应事务、公共组件和命令行工具。
- `test/` 存放 GoogleTest 用例，`test/bdd/` 存放行为测试。
- `docs/` 为项目文档，`demo/Bolt/` 为驱动示例，`docker/` 为镜像脚本，`deps/` 为仓库内依赖。

## 构建、测试与开发命令
- 配置构建目录：`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- 编译项目：`cmake --build build -j28`，编译请使用28核心并行编译。
- 产物位置：`build/lgraph_server`、`build/lgraph_cli`、`build/test_case`
- 单元测试：`./build/test_case`，**运行测试必须先 `cd ./build`，然后在 `./build` 目录内执行，不要在仓库根目录直接运行测试命令**。

## 代码风格与命名约定
- 项目使用 C++17，并默认启用 `-Wall -Wextra`。
- 提交前遵循根目录 `.clang-format`，其基于 Google 风格。
- 可执行 `clang-format -i --style=file <file>`。
- 文件名保持小写加下划线，如 `graph_db.cpp`、`test_vector_index.cpp`。
- 类型名优先使用 PascalCase，变量和函数使用 `lower_snake_case`。
- 新增代码尽量放在对应子系统目录内，避免跨模块堆叠实现。

## 提交与合并请求规范
- 可参考现有风格：`chore: update dockerfile and format codebase`。
- commit message使用英文。
