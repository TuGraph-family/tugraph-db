# 更新内容说明

# 3.4.0 (2023-3-11)

**Breaking Changes:**

1. OLAP Python API support

2. New lgraph_import implementation

3. DataX export support

**Improvements And Bug Fixes:**

1. String 4MB support

2. lgraph_result json format update

3. Other bug fix

# 3.3.4 (2023-1-28)

**Breaking Changes:**

1. None

**Improvements And Bug Fixes:**

1. Fix WAL sync bug

2. Update python rest client

3. Other bug fix

# 3.3.3 (2022-12-23)

**Breaking Changes:**

1. Improve WAL performance

2. Add English Doc

3. Update JWT security issues

**Improvements And Bug Fixes:**

1. Fix edge constraints issues

2. Add ubuntu docker

3. Use pthread in fulltext

4. Other bug fix

# 3.3.2 (2022-11-21)

**Breaking Changes:**

1. Add OGM support

2. Improve UT coverage to 87%

3. Use static library for link

**Improvements And Bug Fixes:**

1. Fix python procedure api doc

2. Update docs

3. Add khop procedure

4. Other bug fix

# 3.3.1 (2022-10-14)

**Breaking Changes:**

1. Refine Graph Analytics Engine

2. Path support in cypher

**Improvements And Bug Fixes:**

1. Fix RWLock Bug

2. Rewrite docs

3. Cleanup Dockerfile

# 3.3.0 (2022-08-05)

**Breaking Changes:**

1. 添加属性级别的访问权限
   
2. db_async选项改名为durable

**Improvements And Bug Fixes:**

1. 使用gtest框架

2. 修复graph为空导致崩溃的问题
   
3. 使用WAL，大幅提高写效率

# 3.2.0 (2022-07-19)

**Breaking Changes:**

1. Edge增加timestamp作为排序键
   
2. 调整少量parallel plugin API

**Improvements And Bug Fixes:**

1. 增加WAL功能，可以提高写操作性能，并降低磁盘性能对系统性能的影响
   
2. Transaction maxreaders由默认值126增大到240，可同时支持更多readers
   
3. LDBC SNB Audit 2022 版本

## 3.1.1 (2022-07-08) 

**Breaking Changes:**

1. 添加Cypher查询内存限制。

2. 添加边索引。

3. 添加全文索引。

4. 优化高可用的启动方式。

5. 图计算系统GeminiLite合并到TuGraph图计算模块。

6. 图计算模块核心代码保护。

7. 图计算模块IDMapping自适应优化。



**Improvements And Bug Fixes:**


1. 修复cypher查询filter丢失的问题。

2. 修复cypher中关键字冲突的问题。

3. 修复点边LABEL冲突的问题。

4. 优化cypher查询：通过vid查找点。

5. 添加cpp/java/python rpc client demo。

6. 添加prometheus支持。

7. 添加DGL支持。

8. 统一procedure和plugin的结果。



## 3.1.0 (2022-01-20)

**Breaking Changes:**

1. 数据模型（schema）调整：增加主键；增加Edge Label的连接Node Labels的限制；
2. 调整导入工具lgraph_import的配置文件格式
3. 导入工具lgraph_import及导出工具lgraph_export支持JSON数据格式
4. 调整Cypher中Schema相关procedures的签名
5. 调整RPC Client接口

**Improvements And Bug Fixes:**

1. 增加DataX工具，支持MySQL、JSON等数据源的数据导入
2. 增加Cypher中用户权限管理相关procedures
3. 更新开发环境Dockerfiles
4. 修复Call plugins时产生的bug
5. Cypher查询语言优化，综合查询效率平均提升10倍


## 3.0.0 (2021-12-14)

**Breaking Changes:**

1. 更换版本升级规则：从2017年的version 1.0.0开始，每年更新一个major version

**Improvements And Bug Fixes:**

1. Cypher性能提升：引入Lazy Materialization机制，优化count DISTINCT性能
2. 源代码组织优化


## 1.20.0 (2021-09-17)

**Breaking Changes:**

1. 修改用户权限管理，增加用户角色管理（注：需要重新导入数据）
2. 服务器配置选项变更：log_file变更为log_dir

**Improvements And Bug Fixes:**

1. 支持Cypher functions对非法参数的处理，支持REGEX、variance
2. 支持数据导入自定义分隔符，允许数据导入发生parser错误时继续
3. 增加python和cpp rest client sdk
4. 优化Cypher不定长扩展的性能，并减少扩展过程内存消耗
5. 修复备份和重新加载plugin功能
6. 更新Web端版本
7. 修复数据导入、Server、Cypher等相关bug若干

## 1.12.0 (2020-09-07)

**Breaking Changes:**

1. 修改批量导入数据的输入格式，分离schema建立和数据导入

**Improvements And Bug Fixes:**

1. 修复HA模式下偶发的内存错误
2. 支持上传zip和cpp格式的plugin
3. 支持clang编译
4. Cypher支持EXPLAIN和PROFILE

## 1.11.0 (2020-07-14)

**Breaking Changes:**

1. 修改内部存储格式，优化读写性能。旧版本数据需要重新导入

**Improvements And Bug Fixes:**

1. Cypher功能完善：支持LDBC SNB interactive查询
2. 支持DeleteLabel和AlterLabel
3. 支持Cypher List Comprehension
4. 增加子图描述字段

## 1.10.0 (2020-06-17)

**Breaking Changes:**

1. 将BIN类型更改为BLOB类型，并支持超过32KB的BLOB

**Improvements And Bug Fixes:**

1. Cypher支持IN {list}，size(string)
2. 对Python plugin使用进程池以改进性能
3. 支持在线修改数据库配置
4. 支持在线备份和增量备份
5. 支持IP白名单

## 1.9.0 (2020-03-10)

**Improvements And Bug Fixes:**

1. Cypher功能完善：支持LDBC SNB interactive-short部分查询
2. lgraph_server增加optimistic_txn选项，该选项为true时cypher将优先使用multi-writer transaction
3. 允许多个LGraph实例共用同一个数据目录(ENABLE_SHARE_DIR)
4. 增加`lgraph_export`工具
5. 重新设计Web界面

## 1.4.5 (2019-10-15)

**Breaking Changes:**
1. lgraph_server的directory, license, web参数默认改为lgraph_server所在目录下的lgraph_db, fma.lic和resouce
2. lgraph_server在-d run（守护进程）模式下的默认工作目录由根目录变为lgraph_server所在目录

**Improvements And Bug Fixes:**
1. 允许lgraph_api中的iterator进行边读边写操作，写操作不会影响其它iterator的正确性
2. Cypher功能完善：OPTIONAL MATCH子句，SET语句, DELETE语句，CASE表达式，算术表达式，WHERE子句支持字符串匹配等
3. 前端页面改进：增加plugin管理页面

## 1.4.4 (2019-08-06)
**Breaking Changes:**
1. 将plugin信息存储在数据库中，因此所有已注册plugin需要删除重新加载
2. 开启python plugin功能
3. python模块更名为lgraph_python，LightningGraph类更名为GraphDB
4. 增加cypher WITH语句的支持

**Improvements And Bug Fixes:**

1. 将CMAKE_BUILD_TYPE默认改回Release以解决性能问题
2. 修复HA模式下snapshot无法顾及plugin的问题
3. 优化TaskTracker带来的性能损失(约2%)
4. 去除C++程序（embedded程序/plugin）编译过程对python库的依赖

## 1.4.3
**Breaking Changes:**
1. 检查用户名，只能为0-9, a-z, A-Z和下划线。
2. 升级license机制，需要用户更新license

**Improvements And Bug Fixes:**
1. 支持任务查询和任务中止
2. 修复windows下lgraph_server无法启动的bug
3. 允许用户在embedded mode中打开多个GraphDB
4. 可视化界面改进

## 1.4.2
**Breaking Changes:**
1. 支持HA模式Cypher查询语言
2. 修改配置选项disable_auth的作用范围，当此选项为true时，服务器不再做权限检查。

**Improvements And Bug Fixes:**
1. 修复前端显示


## 1.4.1
**Breaking Changes:**
1. 暂时关闭python plugin功能，因python plugin在snapshot load时报错

**Improvements And Bug Fixes:**
1. 修复bootstrap问题，同步更新文档

## 1.4.0
**Breaking Changes:**
1. 导入工具`lgraph_import`命令行参数变化，相应的config文件中对边数据的描述格式变化。
2. 数据格式变化，1.4.0之前版本生成的数据需要重新导入。
3. 加载Plugin的方式变化，需要手动导入Plugin文件，相应的REST API变化。
4. Plugin提供只读和读写两种模式，在加载时指定，读写Plugin的调用在HA模式下会被复制。
5. 改进HA模式下的服务器列表及leader状态获取方式，REST报头包含正确的redirect信息。

**Improvements And Bug Fixes:**
1. 修复了REST server在超多client和长请求下死锁的问题。
2. 修复了一些REST API不符合文档的情况。
3. 修复了电网场景下REST API跨域访问的问题。
4. 减少import工具使用的内存量。
