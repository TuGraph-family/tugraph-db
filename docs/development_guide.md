# 业务开发指南
## 连接tugraph-db
### 驱动连接
tugraph-db兼容neo4j的通讯协议，因此可以使用neo4j各个语言的驱动连接tugraph-db的server。

[bolt driver 使用例子](https://github.com/TuGraph-family/tugraph-db/tree/v5.x/demo/Bolt)

### 终端连接
驱动是业务代码里面使用的，对于服务器上终端访问，可以使用lgraph_cli客户端。

[lgraph_cli 使用介绍](https://github.com/TuGraph-family/tugraph-db/blob/master/docs/zh-CN/source/7.client-tools/6.bolt-console-client.md)

### 查询语言
语言为openCypher, tugraph-db实现了openCypher大部分常用的语法。

### Schema
数据模型是schema-free的，不需要提前创建点边schema，数据写入的过程中会自动创建。

## 子图操作
默认会有一个default子图
### 创建子图
```
CALL dbms.graph.createGraph('graph1')
```
### 删除子图
```
CALL dbms.graph.deleteGraph('graph1')
```
### 清空子图数据
```
CALL db.dropDB()
```
### 列出所有子图
```
CALL dbms.graph.listGraph()
```
## 点边类型
### 查看所有的点标签
```
CALL db.labels()
```
### 查看所有的边类型
```
CALL db.relationshipTypes()
```

## 增删改查
### 创建点
```
#为Person类型的点设置一个唯一约束，所有Person类型的点id字段的值是唯一的，为id字段设置唯一索引。
CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');

CREATE (n:Person {id:1, name: 'James'});
CREATE (n:Person {id:2, name: 'Robert'});

#创建一个点，同时具有Person和Actor两个标签
CREATE (n:Person:Actor {id:3, name: 'John'});

#尝试查找具有id:1的Person节点,如果找到了,则将其name更新为'Alice'和age更新为31；如果没有找到，则创建一个新的User节点，其name设置为'Alice'和age设置为30。
MERGE (n:Person {id: 1})
ON CREATE SET n.name = 'Alice', n.age = 30
ON MATCH SET n.name = 'Alice', n.age = 31;
```
### 更新或新增点属性
```
#如果age字段存在，则更新其值；不存在则新增age字段。
MATCH (n:Person {id:1}) SET n.age = 30;
```
### 删除点属性
```
#如果存在age字段，则删除。
MATCH (n:Person {id:1}) REMOVE n.age;
```
### 删除点
```
MATCH (n:Person {id:1}) DELETE n;
```
### 创建边
```
#id为1和2的两个Person节点之间,创建一条LIKE边
MATCH (a:Person {id:1}), (b:Person {id:2}) CREATE (a)-[r:LIKE]->(b);

#创建一条边, 同时设置边的属性
MATCH (a:Person {id:1}), (b:Person {id:2}) CREATE (a)-[r:LIKE {weight:10}]->(b);

#如果id为1和2的两个Person节点之间不存在一条like边，则创建；存在则不创建。
MATCH (a:Person {id:1}), (b:Person {id:2}) MERGE (a)-[r:LIKE]->(b);
```
### 更新或新增边属性
```
#如果边上weight字段存在，则更新其值；不存在则新增weight字段。
MATCH (a:Person {id:1})-[r:LIKE]->(b:Person {id:2}) SET r.weight = 20;
```
### 删除边属性
```
#如果边上存在weight字段，则删除。
MATCH (a:Person {id:1})-[r:LIKE]->(b:Person {id:2}) REMOVE r.weight;
```
### 删除边
```
MATCH (a:Person {id:1})-[r:LIKE]->(b:Person {id:2}) DELETE r;
```

### 其他语法
请参考openCypher语法，或者代码目录test/bdd/features中已经实现的一些语法case

## 向量索引
### 创建向量索引
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
```
### 向量搜索
```
CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node,distance RETURN node,distance;

CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node where node.age > 20 RETURN node;

CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2})
YIELD node where node.age > 20 with node as p
match(p)-[r]->(m) return m;
```
### 删除向量索引
```
CALL db.index.vector.deleteIndex('vector_index');
```
### 查看向量索引
```
CALL db.showIndexes();
```

## 全文检索
### 创建全文索引
```
#为Employee类型的点设置一个唯一约束，所有Employee类型的点name字段的值是唯一的，为name字段设置唯一索引。
CALL db.createUniquePropertyConstraint('employee_name', 'Employee', 'name');

#为Manager类型的点设置一个唯一约束，所有Manager类型的点name字段的值是唯一的，为name字段设置唯一索引。
CALL db.createUniquePropertyConstraint('manager_name', 'Manager', 'name');

#创建全文索引，指定名字是namesAndTeams
CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);

#写入几条点边数据
CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
(lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
(nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
(maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
(lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
(maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
```

### 全文检索
```
CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name;

CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka'
with node
MATCH(node)-[r]->(m)
return m;
```

### 查看全文索引
```
CALL db.showIndexes();
```