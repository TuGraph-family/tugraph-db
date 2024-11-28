# 全文索引

目前只有点类型上可以设置全文索引

创建全文索引

为`Employee`类型点上的`name`字段创建一个全文索引，名字是`namesAndTeams`
```
CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee'], ['name']);
```

另外，也可以为多个点类型联合设置一个全文索引，这样在查询的时候可以同时在多个类型中搜索。

下面创建了一个全文索引，只要点满足如下规则：标签含有`Employee`和`Manager`之一, 字段含有`name`和`team`之一，该点就会被加入到全文索引`namesAndTeams`中。
```
CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
```

使用例子
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

#全文搜索

搜索名字含有nils的点，返回前10个。
第二个参数是模糊查询语句，可以写各种模糊语法，第三个参数是top_k。
CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name;

搜索team是Operations的点，返回前10个，然后再进行图搜索，返回这些点的一跳邻居。
CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka'
with node
MATCH(node)-[r]->(m)
return m;
```
