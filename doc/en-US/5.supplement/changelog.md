# Update the content description

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

1. Add the access permission at the attribute level

2. The db_async option is renamed to durable

**Improvements And Bug Fixes:**

1. Use the gtest framework

2. Fixed crash when graph is empty

3. Use WAL to improve write efficiency

# 3.2.0 (2022-07-19)

**Breaking Changes:**

1. Edge adds timestamp as a sort key

2. Tweak a few parallel plugin apis

**Improvements And Bug Fixes:**

1. Adding WAL improves write performance and reduces the impact of disk performance on system performance

2. Transaction maxreaders is increased from the default value 126 to 240. More readers can be supported

3. LDBC SNB Audit 2022 version

## 3.1.1 (2022-07-08)

**Breaking Changes:**

1. Add the Cypher query memory limit.

2. Add edge indexes.

3. Add a full-text index.

4. Optimize the HA boot mode.

5. Graph computing system GeminiLite merged into TuGraph graph computing module.

6. Figure calculation module core code protection.

7. Adaptive optimization of graph computing module IDMapping.

**Improvements And Bug Fixes:**

1. Rectify the problem that cypher query filter is lost.

2. Fix the keyword conflict problem in cypher.

3. Fix the problem of LABEL conflict at point edges.

4. Optimize the cypher query: use the vid search point.

5. Add cpp/java/python rpc client demo.

6. Add prometheus support.

7. Add DGL support.

8. Unify procedure and plugin results.

## 3.1.0 (2022-01-20)

**Breaking Changes:**

1. Adjust data model (schema) : add primary key; Add the limit of Node Labels on Edge labels.
2. Adjust the configuration file format of the import tool lgraph_import
3. Import tool lgraph_import and export tool lgraph_export support JSON data format
4. Adjust the signatures of schema-related procedures in Cypher
5. Adjust the RPC Client interface

**Improvements And Bug Fixes:**

1. Add the DataX tool to support data import from data sources such as MySQL and JSON
2. procedures for user rights management were added to Cypher
3. Update the development environment Dockerfiles
4. Fix the bug of Call plugins
5. Cypher query language optimization, comprehensive query efficiency increased by 10 times on average

## 3.0.0 (2021-12-14)

**Breaking Changes:**

1. Change the version update rule: Starting from version 1.0.0 in 2017, update a major version every year

**Improvements And Bug Fixes:**

1. Cypher performance improvement: Introduce Lazy Materialization mechanism to optimize the performance of count DISTINCT
2. Source code organization optimization

## 1.20.0 (2021-09-17)

**Breaking Changes:**

1. Modified user rights management and added user role management (Note: Data needs to be imported again)
2. Change the server configuration option: log_file is changed to log_dir

**Improvements And Bug Fixes:**

1. Support Cypher functions in handling invalid parameters and variance
2. Support custom delimiters for data import and allow data import to continue when a parser error occurs
3. Add the python and cpp rest client SDKS
4. Optimize the performance of Cypher indefinite expansion and reduce the memory consumption during expansion
5. Restore the backup and reload the plugin function
6. Update the Web version
7. Fix several bugs related to data import, Server, Cypher, etc

## 1.12.0 (2020-09-07)

**Breaking Changes:**

1. Modify the input format of imported data in batches to separate schema establishment and data import

**Improvements And Bug Fixes:**

1. Rectify memory errors that occur in HA mode
2. Plugins in zip and cpp formats can be uploaded
3. Support clang compilation
4. Cypher supports EXPLAIN and PROFILE

## 1.11.0 (2020-07-14)

**Breaking Changes:**

1. Modify the internal storage format to optimize read and write performance. The data of the old version needs to be imported again

**Improvements And Bug Fixes:**

1. Complete Cypher functions: Supports LDBC SNB interactive query
2. Supports DeleteLabel and AlterLabel
3. Support Cypher List Comprehension
4. Add the subgraph description field

## 1.10.0 (2020-06-17)

**Breaking Changes:**

1. Change the BIN type to BLOB type and support BLOBs larger than 32KB

**Improvements And Bug Fixes:**

1. Cypher supports IN {list}, size(string)
2. Use process pooling for the Python plugin to improve performance
3. Online database configuration modification is supported
4. Supports online backup and incremental backup
5. Support IP address whitelists

## 1.9.0 (2020-03-10)

**Improvements And Bug Fixes:**

1. Complete Cypher functions: Supports LDBC SNB interactive-short queries
2. Add the optimistic_txn option to lgraph_server. If this option is true, cypher will prefered to use multi-writer transaction
3. Allow multiple LGraph instances to share the same data directory (ENABLE_SHARE_DIR)
4. Add 'lgraph_export' tool
5. Redesign the Web interface

## 1.4.5 (2019-10-15)

**Breaking Changes:**

1. lgraph_server directory, license, and web parameters are changed to lgraph_db, fma.lic, and resouce in the directory where lgraph_server resides by default
2. The default working directory of lgraph_server in -d run mode changes from the root directory to the directory where lgraph_server resides

**Improvements And Bug Fixes:**

1. Allow iterators in lgraph_api to read and write simultaneously. Write operations do not affect the correctness of other iterators
2. Complete functions of Cypher: OPTIONAL MATCH clause, SET statement, DELETE statement, CASE expression, arithmetic expression, WHERE clause support string matching, etc
3. Front-end page improvement: add plugin management page

## 1.4.4 (2019-08-06)

**Breaking Changes:**

1. The plugin information is stored in the database, so all registered plugins need to be deleted and reloaded
2. Enable the python plugin
3. The python module is renamed lgraph_python, and the LightningGraph class is renamed GraphDB
4. Add support for the cypher WITH statement

**Improvements And Bug Fixes:**

1. Change the CMAKE_BUILD_TYPE default back to Release to solve the performance problem
2. Fix the plugin issue that snapshot cannot take into account in HA mode
3. Performance loss due to TaskTracker optimization (about 2%)
4. Remove the dependency of C++ program (embedded program /plugin) compilation process on python library

## 1.4.3

**Breaking Changes:**

1. Check the user name. The value can be 0-9, a-z, A-Z and underscores.
2. Upgrade the license mechanism. You need to update the license

**Improvements And Bug Fixes:**

1. Query and abort tasks
2. Fix the bug that lgraph_server fails to start in windows
3. Allow users to open multiple GraphDBs in embedded mode
4. Visual interface improvement

## 1.4.2

**Breaking Changes:**

1. Supports the Cypher query language in HA mode
2. Change the function scope of disable_auth. When the value is true, the server does not perform the permission check.

**Improvements And Bug Fixes:**

1. Restore the front display

## the 1.4.1

**Breaking Changes:**

1. Temporarily disable the python plugin function, because an error is reported in snapshot load

**Improvements And Bug Fixes:**

1. Fix the bootstrap problem and update the document

## 1.4.0

**Breaking Changes:**

1. The command line parameters of the import tool 'lgraph_import' change, and the description format of edge data in the corresponding config file changes.
2. The data format has changed. The data generated earlier than 1.4.0 needs to be imported again.
3. The method of loading the Plugin changes. You need to manually import the Plugin file and the corresponding REST API changes.
4. The Plugin provides read-only and read-write modes. When specified at load time, the call of read-write Plugin will be copied in HA mode.
5. Improve the way to obtain the server list and leader status in HA mode. The REST header contains correct redirect information.

**Improvements And Bug Fixes:**

1. Fixed the REST server deadlock problem with too many clients and long requests.
2. Fixed some REST API inconsistencies.
3. The cross-domain access to REST API in the power grid scenario is fixed.
4. Reduce the memory used by the import tool.
