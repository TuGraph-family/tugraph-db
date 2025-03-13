# Change Log for TuGraph

# 4.5.2 (2025-03-13)

**Breaking Changes:**

1. Add bolt cluster (#847)(#853)(#873)(#867)

**Improvements And Bug Fixes:**

1. Documentation bug fixes and optimization (#753)(#798)(#823)(#830)(#852)(#859)(#868)
2. Compile plugin so with static libstdc++ and libgcc (#821)
3. Add centos9 compile dockerfile (#808)
4. Fix some algorithm result errors (#834)(#870)
5. Initial support for fast schema alteration (#666)
6. Update tugraph-db-browser (#860)
7. lgraph_cli: add failed command to history (#877)

# 4.5.1 (2024-12-03)

**Breaking Changes:**

1. Initially support vector index feature

**Improvements And Bug Fixes:**

1. modify exists execution plan (#670)
2. Fix lgraph_cli hang after a error query (#692)
3. Capture more system signals (#705)
4. Add core dump path (#698)
5. Add missing pair_unique in output of dbms.graph.getGraphSchema() (#707)
6. Fix bolt coredump (#718)
7. support node by label range scan (#722)
8. Skip empty value when adding index (#761)
9. add some string handling functions (#785)
10. Add time print for lgraph_cli (#787)
11. add ReplaceNodeScanWithIndexSeek optimization rule (#801)

# 4.5.0 (2024-09-05)

**Breaking Changes:**

1. Cypher & gql ast are unified, and the query engine is fully upgraded. (#597)(#604)(#644)(#651)
2. The browser front-end architecture is upgraded to support graph analysis. (#658)

**Improvements And Bug Fixes:**

1. Cypher supports parsing emoji symbols. (#657)
2. The format of lgraph_cli output execution plan is optimized. (#648)
3. Optimize the performance of variable-length path queries. (#622)
4. Support using pair unique index to upsert edges. (#636)
5. Cypher supports map type parameters when querying points and edges. (#603)

**Interface Modification:**

1. Add code type to the list plugin return result. (#642)
2. Enrich the result content returned by algo procedure and support output to files. (#634)(#637)(#641)(#643)
3. Support using DataX to import TuGraph. (#629)


# 4.3.2 (2024-07-25)

**Breaking Changes:**

1. Optimize browser architecture and set as the default web.(#608)
2. Add support for vector data type.(#475)

**Improvements And Bug Fixes:**

1. Fix cypher edge property filter does not work.(#559)
2. Reduce the size of rpm package.(#556)
3. Cypher performance optimization.(#570)(#575)
4. Fix the issue that "orderby" variable is not in "return" clause.(#585)
5. Allows to set empty graph in bolt client session.(#589)
6. Documentation fixes.(#563)(#567)(#574)(#569)
7. cypher and gql use the same ast node.(#561)(#581)(#577)

**Interface Modification:**
    
1. Shortest path procedure support multiple edge filter.(#560)

# 4.3.1 (2024-06-13)

**Breaking Changes:**

1. Support non-unique composite index
2. Cypher supports GQL ASTNode parsing links
3. Support bolt data transfer over websocket

**Improvements And Bug Fixes:**

1. Fix the built-in stored procedure algo.native.extract
2. Other bug fixes

**Interface Modification:**

1. The function of adding and deleting procedures is not enabled by default when lgraph_server is started. Enable_procedure must be configured to true to add and delete procedures
2. The java client adds a callCypher interface to return header information

# 4.3.0 (2024-05-21)

**Breaking Changes:**

1. Integrate TuGraph-DB to GraphRAG framework in DB-GPT: https://github.com/eosphoros-ai/DB-GPT/releases/tag/v0.5.6
2. Support using TuGraph-DB in DB-GPT: https://github.com/eosphoros-ai/DB-GPT/releases/tag/v0.5.5
3. Support quick schema changes in detached property model.
4. Support composite index

**Improvements And Bug Fixes:**

1. Add new built-in procedures: upsertVertexByJson, upsertEdgeByJson, createVertexLabelByJson, createEdgeLabelByJson, dropAllVertex.
2. Support user-defined snapshot start time in HA mode.
3. Add new functions for spatial data type.
4. Add development guide doc
5. Other bug fixes.

**Interfaces Modification:**

1. data imported in detached property model by default.
2. new configuration `ha_first_snapshot_start_time` added.

# 4.2.0 (2024-04-11)

**Breaking Changes:**

1. Update the 2024 RoadMap.
2. Document refactor: clearer document directory structure.
3. HA support for the witness role, and the new HA management tool `lgraph_peer`.
4. Bolt supports streaming returns and parameterized queries.

**Improvements And Bug Fixes:**

1. Error code optimization.
2. TuGraph-DB-Browser integration package optimization.
3. Support for fast online full import in HA mode.
4. Support for multiple cpp file uploads in stored procedures.
5. Audit log is now in JSON format.
6. Document error fixes.
7. Other bug fixes.

**Interfaces Modification:**

1. Bolt supports streaming and parameterized queries.
2. Support for multiple cpp files when uploading stored procedures.

# 4.1.0 (2023-12-25)

**Breaking Changes:**

1. Compatibility with the Bolt protocol, supporting Java, JavaScript, Python, Go, Rust, and CLI clients.

2. Support for fast online full import.

3. Implementation of asynchronous task management.

4. Support for spatial data types.

5. Evolution of GQL functionalities.

**Improvements And Bug Fixes:**

1. Graph learning engine now supports labeled graphs.

2. Asynchronous storage snapshot support for high availability.

3. Enhancement of edge indexes, addition of pair-unique uniqueness.

4. Decoupling of the KV layer abstraction.

5. Support for M1 Docker.

6. Optimization of the CI/CD process, significantly reducing CI time.

**Interfaces Modification:**

1. Addition of the Bolt protocol standard.

2. Support for spatial data types.

3. The is_unique parameter in indexes now uses the IndexType enumeration, affecting the AddVertexIndex and AddEdgeIndex interfaces.

# 4.0.1 (2023-9-28)

**Breaking Changes:**

1. Support Temporal order

2. Add 5 algorithms

**Improvements And Bug Fixes:**

1. Python procedure can be killed immediately

2. Extend label name length to 255 bytes

3. Other bug fix

**Interfaces Modification:**

1. Fix temporal keyword in import.conf

# 4.0.0 (2023-9-6)

**Breaking Changes:**

1. ISO GQL support

2. Add 11 graph algorithms

**Improvements And Bug Fixes:**

1. Official m1 docker

2. TuGraph-DB Browser support explore

3. Index bug fix during import

4. Other bug fix

**Interfaces Modification:**

1. Proto version upgraded to 1.2.0

2. ISO GQL query interface in client

# 3.6.0 (2023-8-11)

**Breaking Changes:**

1. High availability support

2. Log system upgrade

**Improvements And Bug Fixes:**

1. Token fix

2. Friendly ci log

3. Mircosecond support for Datatime

4. Other bug fix

# 3.5.1 (2023-7-14)

**Breaking Changes:**

1. Learn engine added

2. Ability to store property in detach mode

3. Rust procedure API

**Improvements And Bug Fixes:**

1. Move fma-common from submodule to include

2. Integrate procedure api doc in readthedocs

3. Add real time count of vertex and edge

4. Other bug fix

# 3.5.0 (2023-6-5)

**Breaking Changes:**

1. POG (aka APOC) support

2. New TuGraph Browser

3. Document reorganize, readthedocs support

**Improvements And Bug Fixes:**

1. Fix edge index bug

2. Update antlr

3. Other bug fix

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

First Open Source Version
