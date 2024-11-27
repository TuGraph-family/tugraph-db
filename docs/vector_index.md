# 向量索引

目前只有点类型上可以设置向量索引

创建向量索引
第一个参数是索引名字，第二个参数是点标签，第三个参数是存向量数据的字段，最后一个参数是个map，里面可以指定向量索引的一些参数。
```
#为Person类型点上的embedding字段创建向量索引，取名vector_index，维度是4, 其他向量参数默认。
CALL db.index.vector.createNodeIndex('vector_index','Person', 'embedding', {dimension:4});
```

使用例子
```
#为Person类型的点设置一个唯一约束，所有Person类型的点id字段的值是唯一的，为id字段设置唯一索引。
CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');

#为Person类型点上的embedding字段创建向量索引，取名vector_index，维度是4, 其他向量参数默认。
CALL db.index.vector.createNodeIndex('vector_index','Person', 'embedding', {dimension:4});

#插入几条点边数据
CREATE (n1:Person {id:1, age:10, embedding: toFloat32List([1.0,1.0,1.0,1.0])})
CREATE (n2:Person {id:2, age:20, embedding: toFloat32List([2.0,2.0,2.0,2.0])})
CREATE (n3:Person {id:3, age:30, embedding: toFloat32List([3.0,3.0,3.0,3.0])})
CREATE (n1)-[r:like]->(n2),
     (n2)-[r:like]->(n3),
     (n3)-[r:like]->(n1);

#向量搜索

#查询与[1.0,2.0,3.0,4.0]相似度最高的前2个点
CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node,distance RETURN node,distance;

#查询与[1.0,2.0,3.0,4.0]相似度最高的前2个点，然后过滤 node.age > 20
CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node where node.age > 20 RETURN node;

#查询与[1.0,2.0,3.0,4.0]相似度最高的前2个点，然后过滤 node.age > 20，最后查这些点的一跳邻居。
CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2})
YIELD node where node.age > 20 with node as p
match(p)-[r]->(m) return m;
```