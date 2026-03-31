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

## 点属性索引

* db.index.createNodeIndex

为点创建属性索引。第三个参数 `properties` 是属性数组，第四个参数 `parameter` 里目前支持：
- `unique`，默认 `false`
```
# 为 Person 类型点的 id 字段创建唯一属性索引
CALL db.index.createNodeIndex('person_id', 'Person', ['id'], {unique:true});

# 为 Person 类型点的 age 字段创建普通属性索引
CALL db.index.createNodeIndex('person_age', 'Person', ['age'], {});

# 为 Person 类型点的 (id, country) 创建复合唯一属性索引
CALL db.index.createNodeIndex('person_id_country', 'Person', ['id', 'country'], {unique:true});
```

* db.index.deleteIndex

删除点属性索引
```
CALL db.index.deleteIndex('person_id');
```

* db.index.queryNodes

按点属性索引做精确匹配查询。

单属性索引时，第二个参数就是索引值；复合索引时，第二个参数需要传和索引属性顺序一致的数组。
```
CALL db.index.queryNodes('person_id', 1) YIELD node RETURN node;
CALL db.index.queryNodes('person_id_country', [1, 'cn']) YIELD node RETURN node;
```

* db.index.rangeQueryNodes

按点属性索引做范围查询，目前是按索引字段顺序做升序扫描。单属性索引时 `lower`/`upper` 直接传值；复合索引时传和索引属性顺序一致的数组。

`parameter` 里支持：
- `left_closed`，默认 `true`
- `right_closed`，默认 `true`
```
CALL db.index.rangeQueryNodes('person_id', 10, 20, {left_closed:true, right_closed:false}) YIELD node RETURN node;
CALL db.index.rangeQueryNodes('person_id_country', [10, 'cn'], [20, 'cn'], {}) YIELD node RETURN node;
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

写入全文索引数据后，系统会在事务提交后立即触发一次wal回放，同时保留后台周期检查作为兜底。

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

* dbms.graph.clearGraph

清空子图数据
```
CALL dbms.graph.clearGraph('graph1');
```

* dbms.graph.listGraph

查看所有子图
```
CALL dbms.graph.listGraph();
```

* db.showIndexes

查看所有的索引
```
CALL db.showIndexes();
```
