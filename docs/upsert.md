# Upsert

upsert语义：如果不存在，则添加；如果存在，则更新。

## 批量 upsert 点

为Person类型点的id字段设置唯一约束，推荐为每个类型的点都设置，这样可以唯一确定一个点。
```
CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
```
下面的语句根据Person的id字段进行查找，如果存在就更新其属性；不存在就创建点。
UNWIND 后面是一个数组，数组里面的每一个元素是个map，map里面是点的属性：id是唯一约束字段，properties是点的其他属性字段值。
UNWIND 后面的数组数据推荐使用驱动的参数化特性传入，不要自己拼接语句。
```
  UNWIND [
    {id: 1, properties:{name: 'Alice', age: 30}},
    {id: 2, properties:{name: 'Bob', age: 25}},
    {id: 3, properties:{name: 'Charlie', age: 35}}
  ] AS record
  MERGE (n:Person {id: record.id})
  ON CREATE SET n += record.properties
  ON MATCH SET n += record.properties;
```

## 批量 upsert 边

下面的语句查找两个Person点之间是否存在一条FRIEND_WITH类型的边，如果存在就更新该边的属性；不存在则创建该条边。
UNWIND 后面是一个数组，数组里面的每一个元素是个map，map里面是点的属性：
from是起点Person的id字段值，to是终点Person的id字段值，properties里面是该边的属性字段值。
UNWIND 后面的数组数据推荐使用驱动的参数化特性传入，不要自己拼接语句。
```
    UNWIND [
      {from: 1, to: 2, properties:{since: 2020}},
      {from: 2, to: 3, properties:{since: 2021}}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel += friendship.properties
    ON MATCH SET rel += friendship.properties;
```

如果边没有属性
```
    UNWIND [
      {from: 1, to: 2},
      {from: 2, to: 3}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)；
```