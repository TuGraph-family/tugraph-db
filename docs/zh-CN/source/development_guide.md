# 业务开发指南
## 连接tugraph-db
### 驱动连接
tugraph-db兼容neo4j的通讯协议，因此可以使用neo4j的驱动连接tugraph-db的server。

[bolt driver 使用介绍](./7.client-tools/5.bolt-client.md)

[bolt driver 使用例子](https://github.com/TuGraph-family/tugraph-db/tree/master/demo/Bolt)

### 终端连接
驱动是业务代码里面使用的，对于服务器上终端访问，可以使用cli客户端。

[console client 使用介绍](./7.client-tools/6.bolt-console-client.md)

## 子图操作
### 创建子图
```
CALL dbms.graph.createGraph('graph1')
```
### 删除子图
```
CALL dbms.graph.deleteGraph('graph1')
```
### 清空子图
#### 删除所有的点边数据和图schema
```
CALL db.dropDB()
```
#### 只删除所有点边数据, 保留图schema
```
CALL db.dropAllVertex()
```
### 查看图schema
```
CALL dbms.graph.getGraphSchema()
```
### 列出所有子图
```
CALL dbms.graph.listGraphs()
```

### 刷新子图文件系统缓存数据
```
CALL db.flushDB()
```

## 点类型操作
### 创建点类型
如下json定义了一个点类型，名字是`node1`。
```json
{
	"label": "node1",
	"primary": "id",
	"type": "VERTEX",
	"detach_property": true,
	"properties": [{
		"name": "id",
		"type": "INT32",
		"optional": false
	}, {
		"name": "name",
		"type": "STRING",
		"optional": false,
		"index": true
	}, {
		"name": "num",
		"type": "INT32",
		"optional": false,
		"index": true,
		"unique": true
	}, {
		"name": "desc",
		"type": "STRING",
		"optional": true
	}]
}

```
把上面这个json序列化成字符串，作为参数传入，建议使用驱动的参数化特性，避免自己拼接语句。
```
CALL db.createVertexLabelByJson($json_data)
```

### 查看点类型schema
```
CALL db.getVertexSchema('node1')
```

### 删除点类型
>该操作会同步删除所有该类型的点数据，数据量大的时候，有时间消耗。

如下例子删除点类型`node1`以及该类型的所有点数据。
```
CALL db.deleteLabel('vertex', 'node1')
```

### 点类型添加字段
>该操作会同步变更所有该类型点的属性数据，数据量大的时候，有时间消耗。

如下例子，对于点类型`node1`，一次添加了两个字段：`field1`，字符串类型，可选，默认值是 `null`; `field2`，`int64`类型，必选，默认值是0.
```
CALL db.alterLabelAddFields('vertex', 'node1', ['field1', string, null ,true], ['field2', int64, 0, false])
```

### 点类型删除字段
>该操作会同步变更所有该类型点的属性数据，数据量大的时候，有时间消耗。

如下例子，对于点类型`node1`，一次删除了两个字段: `field1` 和 `field2`。
```
CALL db.alterLabelDelFields('vertex', 'node1', ['field1', 'field2'])
```

### 点类型添加索引
>该操作会同步构建索引数据，数据量大的时候，有时间消耗。

如下例子，对于点类型`node1`，给`field1`字段添加了一个非唯一索引。
```
CALL db.addIndex('node1', 'field1', false)
```
如下例子，对于点类型`node1`，给`field2`字段添加了一个唯一索引。
```
CALL db.addIndex('node1', 'field2', true)
```

### 点类型删除索引
如下例子，对于点类型`node1`，删除字段`field1`上的索引。
```
CALL db.deleteIndex('node1', 'field1')
```


## 边类型操作
### 创建边类型

如下json定义了一个边的schema，名字是`edge1`。
```json
{
  "label": "edge1",
  "type": "EDGE",
  "detach_property": true,
  "constraints": [
    ["node1", "node2"]
  ]
  "properties": [{
    "name": "id",
    "type": "INT32",
    "optional": false
  }, {
    "name": "name",
    "type": "STRING",
    "optional": false,
    "index": true
  }, {
    "name": "num",
    "type": "INT32",
    "optional": false,
    "index": true,
    "unique": true
  }, {
    "name": "desc",
    "type": "STRING",
    "optional": true
  }]
}
```
把上面这个json序列化成字符串，作为参数传入，建议使用驱动的参数化特性，避免自己拼接语句。
```
CALL db.createEdgeLabelByJson($json_data)
```
### 查看边类型schema
```
CALL db.getEdgeSchema('edge1')
```

### 删除边类型
>该操作会同步删除所有该类型的边，数据量大的时候，有时间消耗。

如下例子，删除边类型`edge1`以及该类型的所有边数据。
```
CALL db.deleteLabel('edge', 'edge1')
```

### 边类型添加字段
>该操作会同步变更所有该类型边的属性数据，数据量大的时候，有时间消耗。

如下例子，对于边类型`edge1`，一次添加了两个字段: `field1`，字符串类型，可选，默认值是 `null`; `field2`，`int64`类型，必选，默认值是`0`.
```
CALL db.alterLabelAddFields('edge', 'edge1', ['field1', string, null ,true], ['field2', int64, 0, false])
```

### 边类型删除字段
>该操作会同步变更所有该类型边的属性数据，数据量大的时候，有时间消耗。

如下操作，对于边类型`edge1`，一次删除了两个字段: `field1` 和 `field2`。
```
CALL db.alterLabelDelFields('edge', 'edge1', ['field1', 'field2'])
```

### 边类型添加索引
>该操作会同步构建索引数据，数据量大的时候，有时间消耗。

如下例子，对于边类型`edge1`，给字段`field1`添加了一个非唯一索引。
```
CALL db.addEdgeIndex('edge1', 'field1', false, false)
```
如下例子，对于边类型`edge1`，给字段`field2`添加了一个唯一索引。
```
CALL db.addEdgeIndex('edge1', 'field2', true, false)
```

### 边类型删除索引
如下例子，对于边类型`edge1`，删除字段`field1`上的索引。
```
CALL db.deleteEdgeIndex('edge1', 'field1')
```

## 实时查看当前点边数据量
如下例子返回所有的点边类型，以及每种类型当前的数据量是多少。

读的是统计数据，轻操作。
```
CALL dbms.meta.countDetail()
```

## 导入数据
### 批量upsert点数据
如果不存在就插入点，如果存在就更新点的属性，根据点的主键字段值判断是否存在。

第二个参数是一个`list`类型，每个`list`里面的元素是个`map`类型，每个`map`里面是点的字段和对应的值。

推荐使用driver里面的参数化特性，第二个参数直接传入一个 `list`结构体，避免自己构造语句。
```
CALL db.upsertVertex('node1', [{id:1, name:'name1'},{id:2, name:'name2'}])
```
### 批量upsert边数据
如果两点之间不存在某条类型的边就插入，如果存在就更新该边的属性。

第四个参数是一个`list`类型，每个数组里面的元素是个`map`类型，每个`map`里面是：边的起点类型主键字段和对应的值、边的终点类型主键字段和对应的值、边类型自身的属性字段和值。每个map里面至少有两个元素。

第二个参数和第三个参数是为第四个参数服务的。分别说明了起点和终点的类型是什么，以及第四个参数中那个字段代表起点主键字段值，那个字段代表终点主键字段值。

推荐使用driver里面的参数化特性，避免自己构造语句。
```
CALL db.upsertEdge('edge1',{type:'node1',key:'node1_id'}, {type:'node2',key:'node2_id'}, [{node1_id:1,node2_id:2,score:10},{node1_id:3,node2_id:4,score:20}])
```

### DataX

https://github.com/ljcui/DataX/tree/bolt 自行编译。

这个DataX实现的 tugraph writer 内部调用的是上面描述的`db.upsertVertex`和`db.upsertEdge`。

### 离线脱机导入数据
如果你有子图的schema以及子图里面所有的点边数据（csv或者json格式），可以利用`lgraph_import`工具离线将这些数据生成图数据。

该方式适合初始阶段，先灌进去一批全量数据。注意server要停机，导入完再启动server，可以看到生成的子图数据。

参考 [lgraph_import 使用介绍](./6.utility-tools/1.data-import.md) 中的**离线全量导入**部分

## 导出数据

### 在线远程流式导出数据
[lgraph_cli 使用介绍](./7.client-tools/6.bolt-console-client.md)

CSV 格式
```
echo "match(n:person) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format csv > output.txt
```
JSON 格式
```
echo "match(n:person) return n.id, n.name;" | lgraph_cli --ip 127.0.0.1 --port 7687 --graph default --user admin --password 73@TuGraph --format json > output.txt

```
### 本地导出整个图的所有数据
[lgraph_export 使用介绍](./6.utility-tools/2.data-export.md)

CSV 格式
```
lgraph_export -d your_db_path -e export_data -g default -f json -u admin -p 73@TuGraph
```
JSON 格式
```
lgraph_export -d your_db_path -e export_data -g default -f json -u admin -p 73@TuGraph
```

### 跨版本迁移数据
[lgraph_export 使用介绍](./6.utility-tools/2.data-export.md)

对于不兼容的版本之间迁移数据，可以使用`lgraph_export`将旧版本server里面的数据全部导出成文本（里面包含图schema以及所有的点边数据），然后再选择合适的方法将这些文本数据导入到新版本。
