Feature: test create
  Scenario: case01
    Given yago graph
    When executing query
      """
      CREATE (passerA:Person {name:'Passerby A', birthyear:1983})
      CREATE (passerB:Person {name:'Passerby B', birthyear:1984})
      CREATE (passerA)-[:MARRIED]->(passerB), (passerB)-[:MARRIED]->(passerA);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 2 edges.'  |

    When executing query
      """
      MATCH (a:Person {name:'Lindsay Lohan'}), (b:Film {title:'The Parent Trap'}) CREATE (a)-[r:DIRECTED]->(b);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 0 vertices, created 1 edges.'  |

    When executing query
      """
      MATCH (a:Person {name:'Lindsay Lohan'})-[r]->(b:Film {title:'The Parent Trap'}) RETURN r;
      """
    Then the result should be, in any order
      | r            |
      | [:DIRECTED]  |
      | [:ACTED_IN{charactername:'Halle/Annie'}]  |
    When executing query
      """
      MATCH (a:Film),(b:City) CREATE (a)-[:BORN_IN]->(b) /* 15 edges */;
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 0 vertices, created 15 edges.'  |
    When executing query
      """
      CREATE (sy:City {name:'Sanya'}) RETURN sy,sy.name;
      """
    Then the result should be, in any order
      | sy                     | sy.name |
      | (:City{name:'Sanya'})  | 'Sanya' |
    When executing query
      """
      MATCH (a:Person {name:'Passerby A'}), (sy:City {name:'Sanya'}) CREATE (a)-[r:BORN_IN]->(sy) RETURN a.name,r,sy.name;
      """
    Then the result should be, in any order
      | a.name       | r         | sy.name |
      | 'PasserbyA'  | [:BORN_IN]| 'Sanya' |
    When executing query
      """
      MATCH (a:Person {name:'Passerby A'}), (sy:City {name:'Sanya'}) WITH a,sy CREATE (a)-[r:BORN_IN]->(sy);
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 0 vertices, created 1 edges.'  |
    When executing query
      """
      CREATE (passerC:Person {name:'Passerby C'})
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |

    When executing query
      """
      MATCH (p:Person {name:'Passerby C'}) RETURN exists(p.birthyear) /* false */;
      """
    Then the result should be, in any order
      | exists(p.birthyear) |
      | false  |
    When executing query
      """
      WITH 'Passerby D' AS x, 2020 AS y CREATE (:Person {name:x, birthyear:y});
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |
    When executing query
      """
      MATCH (a {name:'Passerby A'}) CREATE (b:Person {name:'Passerby E', birthyear:a.birthyear}) return b;
      """
    Then the result should be, in any order
      | b |
      | (:Person {birthyear: 1983, name: 'PasserbyE'})  |
    When executing query
      """
      MATCH (a {name:'Passerby A'}) CREATE (b:Person {name:'Passerby F', birthyear:a.birthyear+24}) return b;
      """
    Then the result should be, in any order
      | b |
      | (:Person {birthyear:2007, name:'PasserbyF'})  |
    When executing query
      """
      MATCH (a {name:'Passerby A'}) CREATE (:Person {name:'Passerby G', birthyear:id(a)});
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |