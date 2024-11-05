Feature: test merge
  Scenario: case01
    Given an empty graph
    And having executed
    """
    CREATE (n:Person {name: 'Liubei', birthyear: 161,gender:1});
    CREATE (n:Person {name: 'Caocao', birthyear: 155,gender:1});
    CREATE (n:Person {name: 'Sunquan', birthyear: 182,gender:1});
    CREATE (n:Person {name: 'Guanyu', birthyear: 160,gender:1});
    CREATE (n:Person {name: 'Zhangfei', birthyear: 153,gender:1});
    CREATE (n:City {name: 'Beijing', area:16410.54, population:2154.2});
    CREATE (n:City {name: 'Shanghai', area:6340.5, population:2423.78});
    """
    When executing query
      """
      MERGE (n:Person {name:'Liubei'}) RETURN n.birthyear, n.gender ;
      """
    Then the result should be, in any order
      | n.birthyear | n.gender |
      | 161  | 1 |

    When executing query
      """
      MERGE (n:Person {name:'Zhugeliang'}) ON CREATE SET n.gender=1,n.birthyear=181 RETURN n.name ;
      """
    Then the result should be, in any order
      | n.name |
      | 'Zhugeliang'  |
    When executing query
      """
      MERGE (n:Person {name:'Liubei'}) ON MATCH SET n.birthyear=2010 RETURN n.birthyear ;
      """
    Then the result should be, in any order
      | n.birthyear |
      | 2010  |
    When executing query
      """
      MERGE(n:Person {name:'Liubei'}) ON CREATE SET n.gender=1 ON MATCH SET n.birthyear=2020 RETURN n.name, n.gender,n.birthyear
      """
    Then the result should be, in any order
      | n.name |n.gender|n.birthyear|
      | 'Liubei' | 1      | 2020      |
    When executing query
      """
      MERGE(n:Person {name:'Huatuo'}) ON CREATE SET n.gender=1 ON MATCH SET n.birthyear=2020 RETURN n.name, n.gender,n.birthyear ;
      """
    Then the result should be, in any order
      | n.name   |n.gender |n.birthyear|
      | 'Huatuo' | 1        | null    |
    When executing query
      """
      MERGE(n:Person {name:'Liubei'}) ON MATCH SET n.gender=0,n.birthyear=2050 RETURN n.name, n.gender,n.birthyear ;
      """
    Then the result should be, in any order
      | n.name   |n.gender |n.birthyear|
      | 'Liubei' | 0        | 2050    |
    When executing query
      """
      MATCH(n:Person {name:'Caocao'}), (m:Person {name:'Sunquan'}) MERGE (n)-[r:Knows{intimacy:0.6}]->(m) RETURN r.intimacy ;
      """
    Then the result should be, in any order
      | r.intimacy|
      | 0.6 |
    When executing query
      """
      MATCH(n:Person {name:'Caocao'}), (m:Person {name:'Sunquan'}) MERGE (n)-[r:Knows]->(m) RETURN r.intimacy ;
      """
    Then the result should be, in any order
      | r.intimacy|
      | 0.6 |
    When executing query
      """
      MATCH (n:Person),(m:City) WHERE n.name='Caocao' AND m.name='Beijing' MERGE (n)-[r:Livein]->(m) RETURN r ;
      """
    Then the result should be, in any order
      | r|
      | [:Livein] |
    When executing query
      """
      MATCH (n:Person {name:'Caocao'}) MERGE (n)-[r:Knows]->(m:Person {name:'Sunquan'})RETURN r ;
      """
    Then the result should be, in any order
      | r|
      | [:Knows {intimacy:0.6}] |
    When executing query
      """
      MATCH (n:Person),(m:City) WHERE n.birthyear >= 160 AND m.name = 'Beijing' MERGE (n)-[r:Livein]->(m) RETURN r ;
      """
    Then the result should be, in any order
      | r|
      | [:Livein] |
      | [:Livein] |
      | [:Livein] |
      | [:Livein] |
    When executing query
      """
      MERGE (n:Person {name:'Caocao'})-[r:Knows]->(m:Person {name:'Caogai'})RETURN r ;
      """
    Then the result should be, in any order
      | r|
      | [:Knows] |
    When executing query
      """
      MERGE (n:Person {name:'Huatuo'}) RETURN n.name ;
      """
    Then the result should be, in any order
      | n.name |
      | 'Huatuo' |
    When executing query
      """
      MERGE (n:Person {name:'Xunyu'}) RETURN n.name ;
      """
    Then the result should be, in any order
      | n.name |
      | 'Xunyu' |
    When executing query
      """
      MERGE (n:Person {name:'Liubei'}) RETURN n.birthyear, n.gender ;
      """
    Then the result should be, in any order
      | n.birthyear |n.gender |
      | 2050 | 0            |
    When executing query
      """
      MERGE (node1: Person {name: 'lisi'}) ON CREATE SET node1.birthyear = 1903 WITH node1 MATCH (node1) WHERE node1.birthyear < 1904 SET node1.birthyear = 1904 RETURN node1.name, node1.birthyear ;
      """
    Then the result should be, in any order
      | node1.name | node1.birthyear |
      | 'lisi' | 1904|
    When executing query
      """
      MERGE (n: Person {name: 'wangwu'}) ON CREATE SET n.birthyear = 1903 ON CREATE SET n.name = 'wangwu2' WITH n MATCH (n) WHERE n.birthyear < 2002 SET n += {birthyear: 2002, name: 'wangwu2'} RETURN n.name, n.birthyear ;
      """
    Then the result should be, in any order
      | n.name | n.birthyear |
      | 'wangwu2' | 2002|
    When executing query
      """
      MERGE (a:Person {name: 'zhangsan'}) SET a.birthyear = 2020 RETURN a.birthyear;
      """
    Then the result should be, in any order
      | a.birthyear |
      | 2020|
    When executing query
      """
      MERGE (a:Person {name: 'zhangsan'}) DELETE a;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'deleted 1 vertices,deleted 0 edges.'  |
    When executing query
      """
      MERGE (a:Person {name: 'zhangsan'}) CREATE (b:Person {name : 'xiaoming'}) RETURN b;
      """
    Then the result should be, in any order
      | b |
      | (:Person{name:'xiaoming'})  |
    When executing query
      """
      MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) RETURN n,m;
      """
    Then the result should be, in any order
      | n | m |
      | (:Person{name:'zhangsan'})  |(:Person{birthyear:1904,name:'lisi'}) |
    When executing query
      """
      MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) CREATE (n)-[r:Knows]->(m) RETURN n, r, m;
      """
    Then the result should be, in any order
      | n | r | m |
      | (:Person{name:'zhangsan'}) |[:Knows] |(:Person{birthyear:1904,name:'lisi'}) |
    When executing query
      """
      MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) MERGE (n)-[r:Knows]->(m) return n, r, m;
      """
    Then the result should be, in any order
      | n | r | m |
      | (:Person{name:'zhangsan'}) |[:Knows] |(:Person{birthyear:1904,name:'lisi'}) |
    When executing query
      """
      MATCH (a:Person {name:'zhangsan'}) SET a.birthyear = 2023 CREATE (b:Person {name:'wangwu'}) RETURN b;
      """
    Then the result should be, in any order
      | b |
      | (:Person{name:'wangwu'}) |
    When executing query
      """
      MATCH (a:Person {name:'zhangsan'}) SET a.birthyear = 2023 MERGE (b:Person {name:'wangwu'}) RETURN b;
      """
    Then the result should be, in any order
      | b |
      | (:Person{name:'wangwu'}) |