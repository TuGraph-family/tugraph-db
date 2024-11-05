Feature: test unwind
  Scenario: case01
    Given an empty graph
    When executing query
      """
      CREATE (a:Person {name:'A', age:13, date:DATE('2023-07-23')})
      CREATE (b:Person {name:'B', age:33, eyes:'blue'})
      CREATE (c:Person {name:'C', age:44, eyes:'blue'})
      CREATE (d:Person {name:'D', eyes:'brown'})
      CREATE (e:Person {name:'E'})
      CREATE (f:Person {name:'F', age:1})
      CREATE (g:Person {name:'G', age:2})
      CREATE (h:Person {name:'H', age:2})
      CREATE (i1:Person {name:'I', age:3})
      CREATE (a)-[:KNOWS {weight:10}]->(b),
             (a)-[:KNOWS {weight:15}]->(c),
             (a)-[:KNOWS {weight:40}]->(d),
             (b)-[:KNOWS {weight:20}]->(e),
             (c)-[:KNOWS {weight:12}]->(e),
             (f)-[:KNOWS {weight:0}]->(g),
             (f)-[:KNOWS {weight:0}]->(h),
             (f)-[:KNOWS {weight:0}]->(i1) ;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 9 vertices, created 8 edges.'  |
    When executing query
      """
      MATCH (n:Person {name:'E'}) SET n.name='X';
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 1 properties.'  |
    When executing query
      """
      MATCH (n:Person {name:'A'}), (m:Person {name:'B'}) SET n.age=50 SET m.age=51;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 2 properties.'  |
    When executing query
      """
      MATCH (n:Person {name:'A'})-[e:KNOWS]->(m:Person) SET n.age=50 SET e.weight=50;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 6 properties.'  |
    When executing query
      """
      MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m.age = 34;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 1 properties.'  |
    When executing query
      """
      MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m.age = id(n);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 1 properties.'  |
    When executing query
      """
      MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m = {age: 33};
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 1 properties.'  |
    When executing query
      """
      match (n) return n,properties(n) /*debug*/;
      """
    Then the result should be, in any order
      | n | properties(n) |
      | (:Person{age:33})  | {age:33} |
      | (:Person{age:51,eyes:'blue',name:'B'})  | {age:51,eyes:'blue',name:'B'} |
      | (:Person{age:44,eyes:'blue',name:'C'})  | {age:44,eyes:'blue',name:'C'} |
      | (:Person{eyes:'brown',name:'D'})  | {eyes:'brown',name:'D'} |
      | (:Person{name:'X'})  | {name:'X'} |
      | (:Person{age:1,name:'F'})  | {age:1,name:'F'} |
      | (:Person{age:2,name:'G'})  | {age:2,name:'G'} |
      | (:Person{age:2,name:'H'})  | {age:2,name:'H'} |
      | (:Person{age:3,name:'I'})  | {age:3,name:'I'} |
    When executing query
      """
      MATCH (n:Person {name:'X'}) SET n += {name:'Y', age:19};
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 2 properties.'  |
    When executing query
      """
      match (n) return n,properties(n) /*debug*/;
      """
    Then the result should be, in any order
      | n | properties(n) |
      | (:Person{age:33})  | {age:33} |
      | (:Person{age:51,eyes:'blue',name:'B'})  | {age:51,eyes:'blue',name:'B'} |
      | (:Person{age:44,eyes:'blue',name:'C'})  | {age:44,eyes:'blue',name:'C'} |
      | (:Person{eyes:'brown',name:'D'})  | {eyes:'brown',name:'D'} |
      | (:Person{age:19,name:'Y'})  | {age:19,name:'Y'} |
      | (:Person{age:1,name:'F'})  | {age:1,name:'F'} |
      | (:Person{age:2,name:'G'})  | {age:2,name:'G'} |
      | (:Person{age:2,name:'H'})  | {age:2,name:'H'} |
      | (:Person{age:3,name:'I'})  | {age:3,name:'I'} |
    When executing query
      """
      MATCH (n {name:'A'})-[r:KNOWS]->(m {name:'B'}) SET r.weight=11;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 0 properties.'  |
    When executing query
      """
      MATCH (n)-[r:KNOWS]->(m) WHERE r.weight=15 SET r += {weight:16};
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 0 properties.'  |
    When executing query
      """
      MATCH (n)-[r:KNOWS]->(m) WHERE r.weight=40 SET r.weight = r.weight + 1;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'set 0 properties.'  |
    When executing query
      """
      match (n)-[r]->(m) return r,properties(r) /*debug*/;
      """
    Then the result should be, in any order
      | r | properties(r) |
      | [:KNOWS{weight:50}]  | {weight:50} |
      | [:KNOWS{weight:50}]  | {weight:50} |
      | [:KNOWS{weight:50}]  | {weight:50} |
      | [:KNOWS{weight:20}]  | {weight:20} |
      | [:KNOWS{weight:12}]  | {weight:12} |
      | [:KNOWS{weight:0}]  | {weight:0} |
      | [:KNOWS{weight:0}]  | {weight:0} |
      | [:KNOWS{weight:0}]  | {weight:0} |
