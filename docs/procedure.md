# 内置的存储过程

## 日志管理

* log.setLevel

在线动态更改日志级别
```
可选日志级别：trace、debug、info、warning、error、critical
CALL log.setLevel('debug');
```
* log.queryLog

在线动态打开或关闭查询日志
```
开启查询日志
CALL log.queryLog(true);
关闭查询日志
CALL log.queryLog(false);
```

## Schema管理
* db.labels

查看目前数据库里面存在的所有点标签
```
CALL db.labels();
```
* db.relationshipTypes

查看目前数据库里面存在的所有边类型
```
CALL db.relationshipTypes();
```

## 点类型唯一约束

* db.createUniquePropertyConstraint

为点的某个字段创建唯一约束
```
为Person类型的点设置一个唯一约束，约束名字是person_id，约束所有Person类型的点id字段的值是唯一的，就是为id字段设置了一个唯一索引。
CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
```

* db.deleteUniquePropertyConstraint

删除点的唯一约束
```
CALL db.deleteUniquePropertyConstraint('person_id');
```

## 点类型全文索引

* db.index.fulltext.createNodeIndex

创建点的全文索引
```
CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee'], ['name']);
```
* db.index.fulltext.queryNodes

点的全文索引查询
```
CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka' RETURN node.name, node.team;
```

* db.index.fulltext.deleteIndex

删除点的全文索引
```
CALL db.index.fulltext.deleteIndex('namesAndTeams');
```

* db.index.fulltext.applyWal

回放点的全文索引WAL日志。

写入全文索引数据后，真正生效有一定的延迟，属于最终一致性读。默认是系统每隔1秒检查一次是否有新增的wal日志，如果有，则读出来进行回放生效。

这个调用是手动触发一次wal的回放。
```
CALL db.index.fulltext.applyWal();
```

## 点类型向量索引
* db.index.vector.createNodeIndex

创建点的向量索引
```
CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
```

* db.index.vector.knnSearchNodes

点的向量索引查询
```
CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node RETURN node;
```

* db.index.vector.deleteIndex

删除点的向量索引
```
CALL db.index.vector.deleteIndex('vector_index');
```

回放点的向量索引WAL日志。

写入向量索引后，真正生效有一定的延迟，属于最终一致性读。默认是系统每隔1秒检查一次是否有新增的wal日志，如果有，则读出来进行回放生效。

这个调用是手动触发一次wal的回放。
```
CALL db.index.vector.applyWal();
```

## 子图管理
* dbms.graph.createGraph

创建子图
```
CALL dbms.graph.createGraph('graph1');
```

* dbms.graph.deleteGraph

删除子图
```
CALL dbms.graph.deleteGraph('graph1');
```

* dbms.graph.listGraph

查看所有子图
```
CALL dbms.graph.listGraph();
```

## 其他
* db.dropDB

清空子图数据
```
CALL db.dropDB();
```

* db.showIndexes

查看所有的索引
```
CALL db.showIndexes();
```