Feature: test delete
  Scenario: case01
    Given an empty graph
    When executing query
      """
      CREATE (a:Person {name:'A', age:13})
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
             (b)-[:KNOWS {weight:25}]->(f),
             (c)-[:KNOWS {weight:12}]->(e),
             (d)-[:KNOWS {weight:4}]->(a),
             (f)-[:KNOWS {weight:0}]->(g),
             (f)-[:KNOWS {weight:0}]->(h),
             (f)-[:KNOWS {weight:0}]->(i1);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 9 vertices, created 10 edges.'  |
    When executing query
      """
      match (n)-[r]->(m) return r, properties(r) /*debug*/;
      """
    Then the result should be, in any order
      | r                    | properties(r) |
      | [:KNOWS{weight:10}]  | {weight:10}   |
      | [:KNOWS{weight:15}]  | {weight:15}   |
      | [:KNOWS{weight:40}]  | {weight:40}   |
      | [:KNOWS{weight:20}]  | {weight:20}   |
      | [:KNOWS{weight:25}]  | {weight:25}   |
      | [:KNOWS{weight:12}]  | {weight:12}   |
      | [:KNOWS{weight:4}]   | {weight:4}   |
      | [:KNOWS{weight:0}]   | {weight:0}   |
      | [:KNOWS{weight:0}]   | {weight:0}   |
      | [:KNOWS{weight:0}]   | {weight:0}   |
    When executing query
      """
      MATCH (n {name:'D'}) DELETE n;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 1 vertices, deleted 2 edges.'  |
    When executing query
      """
      MATCH (n {name:'B'})-[r:KNOWS]->() DELETE r;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 0 vertices, deleted 2 edges.'  |
    When executing query
      """
      MATCH (n:Person {name:'F'})-[r:KNOWS]->(m:Person {name:'I'}) DELETE r;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 0 vertices, deleted 1 edges.'  |
    When executing query
      """
      MATCH (n:Person {name:'A'}),(m:Person {name:'C'}) WITH n,m MATCH (n)-[r]->(m) DELETE r;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 0 vertices, deleted 1 edges.'  |
    When executing query
      """
      match (n)-[r]->(m) return r,properties(r) /*debug*/;
      """
    Then the result should be, in any order
      | r                    | properties(r) |
      | [:KNOWS{weight:10}]  | {weight:10}   |
      | [:KNOWS{weight:12}]  | {weight:12}   |
      | [:KNOWS{weight:0}]   | {weight:0}   |
      | [:KNOWS{weight:0}]   | {weight:0}   |
    When executing query
      """
      MATCH (n:Person {name:'A'}) DELETE n;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 1 vertices, deleted 1 edges.'  |
    When executing query
      """
      MATCH (n:Person {name:'B'}) WITH n DELETE n;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 1 vertices, deleted 0 edges.'  |
    When executing query
      """
      match (n) return n,properties(n) /*debug*/;
      """
    Then the result should be, in any order
      | n | properties(n) |
      | (:Person{age:44,eyes:'blue',name:'C'})  | {age:44,eyes:'blue',name:'C'} |
      | (:Person{name:'E'})  | {name:'E'} |
      | (:Person{age:1,name:'F'})  | {age:1,name:'F'} |
      | (:Person{age:2,name:'G'})  | {age:2,name:'G'} |
      | (:Person{age:2,name:'H'})  | {age:2,name:'H'} |
      | (:Person{age:3,name:'I'})  | {age:3,name:'I'} |