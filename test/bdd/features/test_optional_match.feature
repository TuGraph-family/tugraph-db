Feature: test optional match
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n:Person {name:'NoOne'}) RETURN n;
      '''
    Then the result should be, in any order
      | n |
    When executing query
      '''
      OPTIONAL MATCH (n:Person {name:'NoOne'}) RETURN n;
      '''
    Then the result should be, in any order
      | n    |
      | null |
    When executing query
      '''
      OPTIONAL MATCH (n:City {name:'London'})-[r]->(m) RETURN n.name, r, m;
      '''
    Then the result should be, in any order
      | n.name | r    | m    |
      | 'London' | null | null |
    When executing query
      '''
      OPTIONAL MATCH (n:City {name:'London'})-[r]-(m) RETURN n.name, r, m;
      '''
    Then the result should be, in any order
      | n.name | r                         | m                                                    |
      | 'London' | [:BORN_IN {weight:20.21}] | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | 'London' | [:BORN_IN {weight:20.18}] | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | 'London' | [:BORN_IN {weight:19.93}] | (:Person {name:'Christopher Nolan',birthyear:1970})  |
    When executing query
      '''
      MATCH (n:City {name:'London'}) WITH n.name AS city_name OPTIONAL MATCH (n:Person {name:'NoOne'}) RETURN n.name, city_name;
      '''
    Then the result should be, in any order
      | n.name | city_name |
      | null   | 'London'    |
    When executing query
      '''
      MATCH (n:City) WITH n MATCH (n)-->(m) RETURN n,m;
      '''
    Then the result should be, in any order
      | n | m |
    When executing query
      '''
      MATCH (n:City) WITH n OPTIONAL MATCH (n)-->(m) RETURN n,m;
      '''
    Then the result should be, in any order
      | n                         | m    |
      | (:City {name:'New York'}) | null |
      | (:City {name:'London'})   | null |
      | (:City {name:'Houston'})  | null |
    When executing query
      '''
      MATCH (n:City {name:'London'}) OPTIONAL MATCH (n)-[r]->(m) RETURN n.name, r, m ;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:City {name:'London'}) OPTIONAL MATCH (m:NoLabel) RETURN n.name, m ;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'}) OPTIONAL MATCH (n)-[:ACTED_IN]->(m)-[:NoType]->(l) RETURN n, m, l;
      '''
    Then an Error should be raised