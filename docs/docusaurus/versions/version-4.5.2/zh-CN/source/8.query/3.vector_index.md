# Vector index
## 创建向量索引
如下json定义了一个点类型，名字是`person`, 里面有个字段是`embedding`，类型是`FLOAT_VECTOR`，用来存储向量数据。
目前向量数据只能在点上创建。

```json
{
	"label": "person",
	"primary": "id",
	"type": "VERTEX",
	"properties": [{
		"name": "id",
		"type": "INT32",
		"optional": false
	}, {
		"name": "age",
		"type": "INT32",
		"optional": false
	}, {
		"name": "embedding",
		"type": "FLOAT_VECTOR",
		"optional": false
	}]
}

```
把上面这个json序列化成字符串，作为参数传入，建议使用驱动的参数化特性，避免自己拼接语句。
```
CALL db.createVertexLabelByJson($json_data)
```
给`embedding`字段添加向量索引，第三个参数是个map，里面可以设置一些向量索引的配置参数，如下，`dimension`设置向量维度是4
```
CALL db.addVertexVectorIndex('person','embedding', {dimension: 4});
```

再定义一个边，用来测试，如下json定义了一个边类型，名字是`like`。
```json
{
  "label": "like",
  "type": "EDGE",
  "constraints": [
    ["person", "person"]
  ],
  "properties": []
}
```
把上面这个json序列化成字符串，作为参数传入。
```
CALL db.createEdgeLabelByJson($json_data)
```

写入几条测试数据
```
CREATE (n1:person {id:1, age:10, embedding: [1.0,1.0,1.0,1.0]})
CREATE (n2:person {id:2, age:20, embedding: [2.0,2.0,2.0,2.0]})
CREATE (n3:person {id:3, age:30, embedding: [3.0,3.0,3.0,3.0]})
CREATE (n1)-[r:like]->(n2),
       (n2)-[r:like]->(n3),
       (n3)-[r:like]->(n1);
```
## 向量查询
### KnnSearch
根据向量搜索出点，第四个参数是个map，里面可以指定一些向量搜索的参数。
```
CALL db.vertexVectorKnnSearch('person','embedding', [1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})
yield node return node
```
根据向量搜索出点，返回`age`小于30的
```
CALL db.vertexVectorKnnSearch('person','embedding',[1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})
yield node where node.age < 30 return node
```
根据向量搜索出点，返回age小于30的点，然后再查这些点的一度邻居是谁。
```
CALL db.vertexVectorKnnSearch('person','embedding',[1.0,2.0,3.0,4.0], {top_k:2, hnsw_ef_search:10})
yield node where node.age < 30 with node as p
match(p)-[r]->(m) return m
```
### RangeSearch
根据向量搜索出距离小于10的、age小于30的点，然后再查这些点的一度邻居是谁。
```
CALL db.vertexVectorRangeSearch('person','embedding',[1.0,2.0,3.0,4.0], {radius:10.0, hnsw_ef_search:10})
yield node where node.age < 30 with node as p
match(p)-[r]->(m) return m
```