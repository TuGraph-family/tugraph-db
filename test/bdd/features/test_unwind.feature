Feature: test unwind
  Scenario: case01
    Given yago graph
    When executing query
      """
      WITH [{s: "Michael Redgrave", e: "Vanessa Redgrave"},{s: "Michael Redgrave", e: "Corin Redgrave"}] AS pairs UNWIND pairs AS pair MATCH (n1:Person {name:pair.s})-[r]->(n2:Person{name:pair.e}) RETURN r;
      """
    Then the result should be, in any order
      | r |
      | [:HAS_CHILD]  |
      | [:HAS_CHILD]  |
    When executing query
      """
      UNWIND [1, 2, 3] AS x RETURN x
      """
    Then the result should be, in order
      | x |
      | 1 |
      | 2 |
      | 3 |
    When executing query
      """
      WITH [1, 1, 2, 2] AS coll UNWIND coll AS x RETURN x;
      """
    Then the result should be, in order
      | x |
      | 1 |
      | 1 |
      | 2 |
      | 2 |
    When executing query
      """
      UNWIND [] AS empty RETURN empty, 'literal_that_is_not_returned'
      """
    Then the result should be empty
    When executing query
      """
      UNWIND NULL AS x RETURN x, 'some_literal';
      """
    Then the result should be empty
    When executing query
      """
      UNWIND [1,2] AS x MATCH (n {name:'Houston'}) RETURN x,n;
      """
    Then the result should be, in order
      | x | n |
      | 1 |(:City{name:'Houston'})|
      | 2 |(:City{name:'Houston'})|
    When executing query
      """
      UNWIND [1,2] AS x MATCH (n {name:'Houston'}),(m:Film) RETURN x,n,m;
      """
    Then the result should be, in any order
      | x | n | m |
      | 1 |(:City{name:'Houston'})| (:Film{title:'GoodbyeMr.Chips'}) |
      | 2 |(:City{name:'Houston'})| (:Film{title:'GoodbyeMr.Chips'}) |
      | 1 |(:City{name:'Houston'})| (:Film{title:'BatmanBegins'}) |
      | 2 |(:City{name:'Houston'})| (:Film{title:'BatmanBegins'}) |
      | 1 |(:City{name:'Houston'})| (:Film{title:'HarryPotterandtheSorcerer'sStone'}) |
      | 2 |(:City{name:'Houston'})| (:Film{title:'HarryPotterandtheSorcerer'sStone'}) |
      | 1 |(:City{name:'Houston'})| (:Film{title:'TheParentTrap'}) |
      | 2 |(:City{name:'Houston'})| (:Film{title:'TheParentTrap'}) |
      | 1 |(:City{name:'Houston'})| (:Film{title:'Camelot'}) |
      | 2 |(:City{name:'Houston'})| (:Film{title:'Camelot'}) |

    When executing query
      """
      UNWIND ['Paris','Houston'] AS x MATCH (n {name:x}),(m:Film) RETURN x,n,m;
      """
    Then the result should be, in any order
      | x | n | m |
      |  'Houston' | (:City{name:'Houston'})  |  (:Film{title:'GoodbyeMr.Chips'}) |
      |  'Houston' | (:City{name:'Houston'})  |  (:Film{title:'BatmanBegins'}) |
      |  'Houston' | (:City{name:'Houston'})  |  (:Film{title:'HarryPotterandtheSorcerer'sStone'}) |
      |  'Houston' |  (:City{name:'Houston'}) |  (:Film{title:'TheParentTrap'}) |
      |  'Houston' |  (:City{name:'Houston'}) |  (:Film{title:'Camelot'}) |
    When executing query
      """
      MATCH (c {name:'Houston'}) WITH c MATCH (c)<-[r]-(p) RETURN p;
      """
    Then the result should be, in any order
    | p |
    | (:Person{birthyear:1954,name:'DennisQuaid'}) |
    When executing query
      """
      MATCH (c {name:'Houston'}) WITH c MATCH (p)-[r]->(c) RETURN p;
      """
    Then the result should be, in any order
      | p |
      | (:Person{birthyear:1954,name:'DennisQuaid'}) |
    When executing query
      """
      MATCH (a {name:'Liam Neeson'}) WITH a,'London' AS cid MATCH (c {name:cid}) RETURN a,c;
      """
    Then the result should be, in any order
      | a | c |
    |  (:Person{birthyear:1952,name:'LiamNeeson'}) | (:City{name:'London'})  |
    When executing query
      """
      MATCH (a {name:'Liam Neeson'}) WITH a,['London','Houston'] AS cids UNWIND cids AS cid MATCH (c {name:cid}) RETURN a,count(c);
      """
    Then the result should be, in any order
      | a | count(c) |
      |  (:Person{birthyear:1952,name:'LiamNeeson'}) | 2  |
    When executing query
      """
      MATCH (a {name:'Liam Neeson'}),(b {name:'Dennis Quaid'}) WITH a,b,['London','Houston'] AS cids UNWIND cids AS cid MATCH (c {name:cid}) RETURN a,b,count(c);
      """
    Then the result should be, in any order
      | a |b| count(c) |
      |  (:Person{birthyear:1952,name:'LiamNeeson'}) | (:Person{birthyear:1954,name:'DennisQuaid'})  | 2 |
    When executing query
      """
      MATCH (a {name:'Dennis Quaid'}) WITH a,['London','Houston'] AS cids UNWIND cids AS cid MATCH (c {name:cid})<-[]-(a) RETURN a,count(c);
      """
    Then the result should be, in any order
      | a | count(c) |
      |  (:Person{birthyear:1954,name:'DennisQuaid'}) | 1  |
    When executing query
      """
      MATCH (a {name:'Liam Neeson'}) WITH a,['London','Houston'] AS cids UNWIND cids AS cid MATCH (c {name:cid})<-[]-()-[:MARRIED]->(a) RETURN a,count(c);
      """
    Then the result should be, in any order
      | a | count(c) |
      |  (:Person{birthyear:1952,name:'LiamNeeson'}) | 1  |
    When executing query
      """
      MATCH (c {name:'Houston'}) WITH c MATCH (p:Person {name:'Liam Neeson'}) CREATE (c)-[:HAS_CHILD]->(p);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      |  'created 0 vertices,created 1 edges.' |
    When executing query
      """
      MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer1', birthyear:2002})-[r:BORN_IN]->(c) RETURN p,r,c;
      """
    Then the result should be, in any order
      | p | r | c |
      |  (:Person{birthyear:2002,name:'passer1'}) |[:BORN_IN] |(:City{name:'Houston'}) |
    When parameters are:
      | personIds |
      | ['Liam Neeson','Dennis Quaid','Roy Redgrave'] |
    And executing query
      """
      MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer2', birthyear:2002})-[r:BORN_IN]->(c) WITH p UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      |  'created 1 vertices,created 4 edges.' |
    When parameters are:
      | personIds |
      | ['Liam Neeson','Dennis Quaid','Roy Redgrave'] |
    And executing query
      """
      MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer3', birthyear:2002})-[r:BORN_IN]->(c) WITH p UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q) WITH p UNWIND ['Liam Neeson'] AS sId MATCH (s:Person {name:sId}) CREATE (p)-[:DIRECTED]->(s) /* 1,7 */;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      |  'created 1 vertices,created 7 edges.' |
    When parameters are:
      | personIds |
      | ['Liam Neeson','Dennis Quaid','Roy Redgrave'] |
    And executing query
      """
      MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer4', birthyear:2002})-[r:BORN_IN]->(c) WITH p UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q) WITH p UNWIND [] AS sId MATCH (s:Person {name:sId}) CREATE (p)-[:DIRECTED]->(s) /* 1,4 */;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      |  'created 1 vertices,created 4 edges.' |
    When executing query
      """
      WITH [1, 1, 2, 2] AS coll UNWIND coll AS x WITH x RETURN collect(x);
      """
    Then the result should be, in any order
      | collect(x) |
      |  [1,1,2,2] |
    When executing query
      """
      WITH [1, 1, 2, 2] AS coll UNWIND coll AS x WITH x RETURN collect(DISTINCT x);
      """
    Then the result should be, in any order
      | collect(DISTINCT x) |
      |  [1,2] |
    When executing query
      """
      CREATE (:City {name:'Shanghai'}), (:City {name:'Zhongshan'}), (:Person {name:'Zhongshan'});
      """
    Then the result should be, in any order
      | <SUMMARY> |
      |  'created 3 vertices,created 0 edges.' |
    When executing query
      """
      UNWIND ['Zhongshan'] AS x WITH x MATCH (a {name:x}) RETURN a,a.name;
      """
    Then the result should be, in any order
      | a | a.name |
      | (:City{name:'Zhongshan'}) |  'Zhongshan' |
      | (:Person{name:'Zhongshan'}) |  'Zhongshan' |
    When executing query
      """
      UNWIND ['Zhongshan', 'Shanghai'] AS x WITH x MATCH (a {name:x}) RETURN a,a.name;
      """
    Then the result should be, in any order
      | a | a.name |
      | (:City{name:'Zhongshan'}) |  'Zhongshan' |
      | (:Person{name:'Zhongshan'}) |  'Zhongshan' |
      | (:City{name:'Shanghai'}) |  'Shanghai' |
    When executing query
      """
      UNWIND [{name: 'Alice', age: 30},{name: 'Bob', age: 25},{name: 'Charlie', age: 35}] AS user CREATE (u:User {name: user.name, age: user.age});
      """
    Then the result should be, in any order
      | <SUMMARY>                            |
      | 'created 3 vertices, created 0 edges.' |