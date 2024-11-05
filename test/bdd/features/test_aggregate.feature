Feature: test aggregate
  Scenario: case01
    Given an empty graph
    And having executed
      """
      CREATE (a:Person {name:'A', age:13})
      CREATE (b:Person {name:'B', age:33, eyes:'blue'})
      CREATE (c:Person {name:'C', age:44, eyes:'blue'})
      CREATE (d:Person {name:'D', eyes:'brown'})
      CREATE (e:Person {name:'E'})
      CREATE (a)-[:KNOWS]->(b),
            (a)-[:KNOWS]->(c),
            (a)-[:KNOWS]->(d),
            (b)-[:KNOWS]->(e),
            (c)-[:KNOWS]->(e)
      """
    When executing query
      """
      MATCH (n:Person) RETURN avg(n.age) /* 30.0 */;
      """
    Then the result should be, in any order
      | avg(n.age) |
      | 30.0  |
    When executing query
      """
      MATCH (n { name: 'A' })-->(x) RETURN count(x) /* 3 */;
      """
    Then the result should be, in any order
      | count(x) |
      | 3  |
    When executing query
      """
      MATCH (n:Person) RETURN count(n.age) /* 3 */;
      """
    Then the result should be, in any order
      | count(n.age) |
      | 3  |
    When executing query
      """
      MATCH (n:Person) RETURN max(n.age) /* 44 */;
      """
    Then the result should be, in any order
      | max(n.age) |
      | 44.0  |
    When executing query
      """
      MATCH (n:Person) RETURN min(n.age) /* 13 */;
      """
    Then the result should be, in any order
      | min(n.age) |
      | 13.0  |
    When executing query
      """
      MATCH (n:Person) RETURN percentileCont(n.age, 0.4) /* 29 */;
      """
    Then the result should be, in any order
      | percentileCont(n.age, 0.4) |
      | 29.0  |
    When executing query
      """
      MATCH (n:Person) RETURN percentileDisc(n.age, 0.5) /* 33 */;
      """
    Then the result should be, in any order
      | percentileDisc(n.age, 0.5) |
      | 33.0  |
    When executing query
      """
      MATCH (n:Person) RETURN stDev(n.age) /* 15.716234 */;
      """
    Then the result should be, in any order
      | stDev(n.age) |
      | 15.716233645501712  |
    When executing query
      """
      MATCH (n:Person) RETURN stDevP(n.age) /* 12.832251 */;
      """
    Then the result should be, in any order
      | stDevP(n.age) |
      | 12.832251036613439  |
    When executing query
      """
      MATCH (n:Person) RETURN variance(n.age);
      """
    Then the result should be, in any order
      | variance(n.age) |
      | 247.0  |
    When executing query
      """
      MATCH (n:Person) RETURN varianceP(n.age);
      """
    Then the result should be, in any order
      | varianceP(n.age) |
      | 164.66666666666666  |
    When executing query
      """
      MATCH (n:Person) RETURN collect(n.age) /* 13,33,44 */;
      """
    Then the result should be, in any order
      | collect(n.age) |
      | [13,33,44]  |
    When executing query
      """
      MATCH (n:Person) RETURN collect([n.name,n.age]) /* [[A, 13], [B, 33], [C, 44], [D, null], [E, null]] */;
      """
    Then the result should be, in any order
      | collect([n.name,n.age]) |
      | [['A',13],['B',33],['C',44],['D',null],['E',null]]  |
    When executing query
      """
      MATCH (n {name: 'A'})-[]->(x) RETURN labels(n), n.age, count(*) /* Person,13,3.000000 */;
      """
    Then the result should be, in any order
      | labels(n) | n.age | count(*) |
      | ['Person'] | 13 | 3 |
    When executing query
      """
      MATCH (n {name: 'A'})-[]->(x) RETURN labels(n), n, count(*) /* Person,V[0],3.000000 */;
      """
    Then the result should be, in any order
      | labels(n) | n | count(*) |
      | ['Person'] | (:Person{age:13,name:'A'}) | 3 |
    When executing query
      """
      MATCH (n {name: 'A'})-[r]->() RETURN type(r), count(*) /* KNOWS,3.00000 */;
      """
    Then the result should be, in any order
      | type(r) | count(*) |
      | 'KNOWS' | 3 |
    When executing query
      """
      MATCH (n:Person) WHERE n.age = 13 OR n.age > 40 RETURN count(n) AS nCount;
      """
    Then the result should be, in any order
      | nCount |
      | 2 |