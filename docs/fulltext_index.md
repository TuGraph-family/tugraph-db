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
#清空子图数据
CALL db.dropDB();

#为Employee类型的点设置一个唯一属性索引，所有Employee类型的点name字段的值是唯一的。
CALL db.index.createNodeIndex('employee_name', 'Employee', ['name'], {unique:true});

#为Manager类型的点设置一个唯一属性索引，所有Manager类型的点name字段的值是唯一的。
CALL db.index.createNodeIndex('manager_name', 'Manager', ['name'], {unique:true});

#创建全文索引，指定名字是namesAndTeams
CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);

#写入几条点边数据
CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
(lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
(nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
(maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
(lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
(maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);

#这里手动触发了一次回放全文索引的wal，该操作可选。默认系统会在事务提交后立即触发回放，同时保留后台周期检查作为兜底。
CALL db.index.fulltext.applyWal();

#全文搜索

搜索名字含有nils的点，返回前10个。
第二个参数是模糊查询语句，可以写各种模糊语法，第三个参数是top_k。
CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name;

全文索引默认支持中文分词，查询语法不变。对于无空格中文文本，可以直接使用中文词语查询，例如：
```
CALL db.index.fulltext.queryNodes("medicalTerms", "恶性肿瘤", 10) YIELD node, score RETURN node;
```

搜索team是Operations的点，返回前10个，然后再进行图搜索，返回这些点的一跳邻居。
CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka'
with node
MATCH(node)-[r]->(m)
return m;
```
