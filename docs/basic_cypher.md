# 一些基本的cypher语句

## 点操作
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

## 边操作
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
请自行搜索OpenCypher语法，如果遇到没有支持的语法，请在tugraph-db github仓库上提issue。