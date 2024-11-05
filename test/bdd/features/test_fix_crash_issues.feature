Feature: test fix crash issues
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'}) WITH n AS aa RETURN aa.name;
      '''
    Then the result should be, in any order
      | aa.name     |
      | 'Liam Neeson' |
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'})-[e]->(m) WITH e AS aa RETURN aa;
      '''
    Then the result should be, in any order
      | aa                                         |
      | [:MARRIED]                              |
      | [:ACTED_IN {charactername:'Henri Ducard'}] |
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'}) SET n.birthyear=2052 RETURN n.birthyear;
      '''
    Then the result should be, in any order
      | n.birthyear |
      | 2052        |
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'}),(m:Person {name:'Richard Harris'}) SET n.birthyear=2152 RETURN n.birthyear,m.birthyear;
      '''
    Then the result should be, in any order
      | n.birthyear | m.birthyear |
      | 2152        | 1930        |
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'}),(m:Person {name:'Richard Harris'}) SET n.birthyear=2252,m.birthyear=2230 RETURN n.birthyear,m.birthyear;
      '''
    Then the result should be, in any order
      | n.birthyear | m.birthyear |
      | 2252        | 2230        |
    When executing query
      '''
      MERGE (z3:Person {name:'zhang3'}) ON CREATE SET z3.birthyear=2021 ON MATCH SET z3.birthyear=2022 WITH z3 MERGE (z4:Person {name:'zhang4'}) ON CREATE SET z4.birthyear=2021 ON MATCH SET z4.birthyear=2022 WITH z3,z4 CREATE (z3)-[:HAS_CHILD]->(z4);
      '''
    Then the result should be, in any order
      | <SUMMARY>                            |
      | 'created 2 vertices, created 1 edges.' |
    When executing query
      '''
      MATCH (z3:Person {name:'zhang3'})-[r]->(z4:Person {name:'zhang4'}) RETURN r;
      '''
    Then the result should be, in any order
      | r               |
      | [:HAS_CHILD] |
    When executing query
      '''
      MERGE (z3:Person {name:'zhang3'}) ON CREATE SET z3.birthyear=2021 ON MATCH SET z3.birthyear=2022 WITH z3 MERGE (z4:Person {name:'zhang4'}) ON CREATE SET z4.birthyear=2021 ON MATCH SET z4.birthyear=2022 WITH z3,z4 CREATE (z3)-[r:HAS_CHILD]->(z4) RETURN z3,z4,r;
      '''
    Then the result should be, in any order
      | z3                                       | z4                                       | r               |
      | (:Person {name:'zhang3',birthyear:2022}) | (:Person {name:'zhang4',birthyear:2022}) | [:HAS_CHILD ] |
    When executing query
      '''
      MATCH (z3:Person {name:'zhang3'})-[r]->(z4:Person {name:'zhang4'}) RETURN r;
      '''
    Then the result should be, in any order
      | r               |
      | [:HAS_CHILD] |
      | [:HAS_CHILD ] |
    When executing query
      '''
      MATCH (m:City) RETURN collect(m.name) + [1,2];
      '''
    Then the result should be, in any order
      | collect(m.name) + [1,2]             |
      | ['New York','London','Houston',1,2] |
    When executing query
      '''
      WITH [1,2] AS nn MATCH (m:City) RETURN collect(m.name) + nn;
      '''
    Then the result should be, in any order
      | collect(m.name) + nn                |
      | ['New York','London','Houston',1,2] |
    When executing query
      '''
      MATCH (n:City) WITH collect(n.name) AS nn MATCH (m:City) RETURN collect(m.name) + nn;
      '''
    Then the result should be, in any order
      | collect(m.name) + nn                                          |
      | ['New York','London','Houston','New York','London','Houston'] |
    When executing query
      '''
      MATCH (n:Person) RETURN -n.birthyear LIMIT 3;
      '''
    Then the result should be, in any order
      | -n.birthyear |
      | -1910        |
      | -1908        |
      | -1937        |
    When executing query
      '''
      MATCH (n:Person) RETURN -sum(n.birthyear);
      '''
    Then the result should be, in any order
      | -sum(n.birthyear) |
      | -29863.0          |
    When executing query
      '''
      MATCH (n) -[r:HAS_CHILD * 2 ]->(m) RETURN n,m;
      '''
    Then the result should be, in any order
      | n                                                  | m                                                    |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Corin Redgrave',birthyear:1939})     |
    When executing query
      '''
      MATCH (n) -[r:HAS_CHILD * .. ]->(m) RETURN n,m ;
      '''
    Then the result should be, in any order
      | n                                                  | m                                                    |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'zhang3',birthyear:2022})           | (:Person {name:'zhang4',birthyear:2022})             |
      | (:Person {name:'zhang3',birthyear:2022})           | (:Person {name:'zhang4',birthyear:2022})             |
    When executing query
      '''
      WITH '1' as s UNWIND ['a','b'] as k RETURN s,k;
      '''
    Then the result should be, in any order
      | s | k |
      | '1' | 'a' |
      | '1' | 'b' |
    When executing query
      '''
      WITH '1' as s UNWIND ['a','b']+s as k RETURN s,k;
      '''
    Then the result should be, in any order
      | s | k |
      | '1' | 'a' |
      | '1' | 'b' |
      | '1' | '1' |
    When executing query
      '''
      MATCH (n:Person)-[]->(m:Film) WITH n.name AS nname, collect(id(m)) AS mc MATCH (n:Person {name: nname})<-[]-(o) WITH n.name AS nname, mc, collect(id(o)) AS oc UNWIND mc+oc AS c RETURN c;
      '''
    Then the result should be, in any order
      | c  |
      | 18 |
      | 6  |
      | 17 |
      | 11 |
      | 1  |
      | 21 |
      | 1  |
      | 2  |
      | 20 |
      | 3  |
      | 5  |
    When executing query
      '''
      MATCH (m:Person)-[r:BORN_IN]->(n:City) WHERE n.name = 'London' and r.weight >= 1 and r.weight <= 100 RETURN sum(r.weight);
      '''
    Then the result should be, in any order
      | sum(r.weight) |
      | 60.32         |
    When executing query
      '''
      MATCH (n:City) RETURN collect(n.name) + n.name;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:Person) RETURN NOT n.nam;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:Person) RETURN -n.name;
      '''
    Then an Error should be raised
    When executing query
      '''
      REMOVE a.name;
      '''
    Then an Error should be raised
    When executing query
      '''
      SET a :MyLabel;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:Person) WITH n,n.name RETURN n.name;
      '''
    Then an Error should be raised
    When executing query
      '''
      WITH * MERGE(n:Person) RETURN n;
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN * UNION RETURN *;
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN * UNION RETURN 1 AS a;
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN 2 AS b UNION RETURN 1 AS a;
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN 2 AS b UNION RETURN 1 AS a, 3 AS c;
      '''
    Then an Error should be raised
    When executing query
      '''
      DELETE [];
      '''
    Then an Error should be raised
    When executing query
      '''
      DELETE [x in [1, 2, 3] | x];
      '''
    Then an Error should be raised
    When executing query
      '''
      DELETE TRUE;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n:Person {name:'Liam Neeson'}), (m:Person {name:'Liam Neeson'}), (o:Person {name:'Liam Neeson'}) WHERE custom.myadd('asd')='1' RETURN 1;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (movie)<-[r]-(n) WITH n,n MATCH (n1) RETURN n1 LIMIT 1;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (movie)<-[r]-(n) return n,n limit 1;
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (n) RETURN n LIMIT 5;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   |
      | (:Person {name:'Liam Neeson',birthyear:2252})      |
    When executing query
      '''
      MATCH p=(n)-[e]->(m) RETURN p LIMIT 5;
      '''
    Then the result should be, in any order
      | p                                                                                                                      |
      | <(:Person {name:'Rachel Kempson',birthyear:1910})-[:HAS_CHILD]->(:Person {name:'Vanessa Redgrave',birthyear:1937})>   |
      | <(:Person {name:'Rachel Kempson',birthyear:1910})-[:HAS_CHILD]->(:Person {name:'Corin Redgrave',birthyear:1939})>     |
      | <(:Person {name:'Rachel Kempson',birthyear:1910})-[:MARRIED]->(:Person {name:'Michael Redgrave',birthyear:1908})>     |
      | <(:Person {name:'Michael Redgrave',birthyear:1908})-[:HAS_CHILD]->(:Person {name:'Vanessa Redgrave',birthyear:1937})> |
      | <(:Person {name:'Michael Redgrave',birthyear:1908})-[:HAS_CHILD]->(:Person {name:'Corin Redgrave',birthyear:1939})>   |