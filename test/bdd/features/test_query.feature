Feature: test query
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[:ACTED_IN]->(m) RETURN n,m.title;
      '''
    Then the result should be, in any order
      | n                                                  | m.title |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) | 'Camelot' |
    When executing query
      '''
      MATCH (:Person {name:'Vanessa Redgrave'})-[:ACTED_IN]->(movie) return movie.title;
      '''
    Then the result should be, in any order
      | movie.title |
      | 'Camelot'     |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[relatedTo]-(b) RETURN b,relatedTo;
      '''
    Then the result should be, in any order
      | b                                                    | relatedTo                               |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | [:HAS_CHILD ]                         |
      | (:City {name:'London'})                              | [:BORN_IN {weight:20.21}]               |
      | (:Film {title:'Camelot'})                            | [:ACTED_IN {charactername:'Guenevere'}] |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | [:HAS_CHILD ]                         |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | [:HAS_CHILD ]                         |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD]-(b) RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
      | 'Michael Redgrave'   |
    When executing query
      '''
      MATCH (m:Film {title:'Batman Begins'})<-[:DIRECTED]-(directors) RETURN directors.name;
      '''
    Then the result should be, in any order
      | directors.name    |
      | 'Christopher Nolan' |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[]-(neighbors) RETURN neighbors;
      '''
    Then the result should be, in any order
      | neighbors                                            |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:City {name:'London'})                              |
      | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
    When executing query
      '''
      MATCH (n:Person {name:'Lindsay Lohan'})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors) RETURN coActors.name;
      '''
    Then the result should be, in any order
      | coActors.name      |
      | 'Natasha Richardson' |
      | 'Dennis Quaid'       |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[]->()<-[]-(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Christopher Nolan',birthyear:1970})  |
      | (:Person {name:'Richard Harris',birthyear:1930})     |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[]->()<-[]-(m) RETURN DISTINCT m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Christopher Nolan',birthyear:1970})  |
      | (:Person {name:'Richard Harris',birthyear:1930})     |
    When executing query
      '''
      MATCH (na)-[]->(nb)-[]->(nc) RETURN na,nb,nc;
      '''
    Then the result should be, in any order
      | na                                                   | nb                                                   | nc                                                   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:City {name:'London'})                              |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Corin Redgrave',birthyear:1939})     | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Film {title:'Goodbye Mr. Chips'})                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:City {name:'London'})                              |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Corin Redgrave',birthyear:1939})     | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910})     | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:City {name:'London'})                              |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Film {title:'The Parent Trap'})                    |
      | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:City {name:'London'})                              |
      | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Film {title:'The Parent Trap'})                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Film {title:'Batman Begins'})                      |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Film {title:'Goodbye Mr. Chips'})                  |
    When executing query
      '''
      MATCH (na:Person)-[:HAS_CHILD]->(nb)-[:MARRIED]->(nc) RETURN na,nb,nc;
      '''
    Then the result should be, in any order
      | na                                                 | nb                                                   | nc                                               |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})    |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Rachel Kempson',birthyear:1910}) |
    When executing query
      '''
      MATCH (m:Film {title:'Camelot'})<-[r:ACTED_IN]-(n) RETURN n.name,r.charactername;
      '''
    Then the result should be, in any order
      | n.name           | r.charactername |
      | 'Vanessa Redgrave' | 'Guenevere'       |
      | 'Richard Harris'   | 'King Arthur'     |
    When executing query
      '''
      MATCH (n)-[relatedTo]->(vanessa:Person {name:'Vanessa Redgrave'}) RETURN n,relatedTo;
      '''
    Then the result should be, in any order
      | n                                                  | relatedTo       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | [:HAS_CHILD ] |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | [:HAS_CHILD ] |
    When executing query
      '''
      MATCH (n)<-[relatedTo]-(vanessa:Person {name:'Vanessa Redgrave'}) RETURN n,relatedTo;
      '''
    Then the result should be, in any order
      | n                                                    | relatedTo                               |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | [:HAS_CHILD ]                         |
      | (:City {name:'London'})                              | [:BORN_IN {weight:20.21}]               |
      | (:Film {title:'Camelot'})                            | [:ACTED_IN {charactername:'Guenevere'}] |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[]->(b:Person) RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b:Person) RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
      | 'Michael Redgrave'   |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
      | 'Michael Redgrave'   |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person AND b.birthyear >= 1910 RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person OR b:City RETURN b.name;
      '''
    Then the result should be, in any order
      | b.name             |
      | 'Natasha Richardson' |
      | 'London'             |
      | 'Rachel Kempson'     |
      | 'Michael Redgrave'   |
    When executing query
      '''
      MATCH (n:Person {name:'Lindsay Lohan'})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors)-[:BORN_IN]->(city) RETURN coActors.name,city.name;
      '''
    Then the result should be, in any order
      | coActors.name      | city.name |
      | 'Natasha Richardson' | 'London'    |
      | 'Dennis Quaid'       | 'Houston'   |
    When executing query
      '''
      MATCH (a)-->(b)-->(c)<--(d) USING JOIN ON c WHERE a.uid > 1 AND d.uid > 2 AND b.uid < 3 AND c.uid < 4 RETURN d;
      '''
    Then the result should be, in any order
      | d |
    When executing query
      '''
      MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > 1 AND d.uid > 2 AND b.uid < 3 AND c.uid < 4 RETURN d;
      '''
    Then the result should be, in any order
      | d |
    When executing query
      '''
      MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > d.uid AND b.uid < c.uid RETURN d;
      '''
    Then the result should be, in any order
      | d |
    When executing query
      '''
      MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > d.uid AND b.uid < c.uid AND a.uid > b.uid RETURN d;
      '''
    Then the result should be, in any order
      | d |
    When executing query
      '''
      MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 20.18 RETURN m;
      '''
    Then the result should be, in any order
      | m                        |
      | (:City {name:'Houston'}) |
      | (:City {name:'London'})  |
    When executing query
      '''
      MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 20.18 AND m.name <> 'Houston' RETURN m;
      '''
    Then the result should be, in any order
      | m                       |
      | (:City {name:'London'}) |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = a.title RETURN n;
      '''
    Then the result should be, in any order
      | n |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE a.role = 'Iron Man' RETURN n;
      '''
    Then the result should be, in any order
      | n |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = 'Vanessa Redgrave' RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE m.title = 'Camelot' RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Richard Harris',birthyear:1930})   |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = a.title AND m.title = 'Camelot' RETURN n;
      '''
    Then the result should be, in any order
      | n |
    When executing query
      '''
      MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name =  'Vanessa Redgrave' AND m.title = a.title RETURN n;
      '''
    Then the result should be, in any order
      | n |
    When executing query
      '''
      MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 19.2 OR b.weight > 20.6 RETURN m;
      '''
    Then the result should be, in any order
      | m                         |
      | (:City {name:'Houston'})  |
      | (:City {name:'New York'}) |
    When executing query
      '''
      MATCH (n:Person)-[b:BORN_IN]->(m) WHERE (b.weight + b.weight) < 38.4 OR b.weight > 20.6 RETURN m;
      '''
    Then the result should be, in any order
      | m                         |
      | (:City {name:'Houston'})  |
      | (:City {name:'New York'}) |
    When executing query
      '''
      MATCH (a)-[e]->(b) WHERE a.name='Liam Neeson' and b.title<>'' and (e.charactername='Henri Ducard' or e.relation = '') RETURN a,e,b;
      '''
    Then the result should be, in any order
      | a                                             | e                                          | b                               |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | [:ACTED_IN {charactername:'Henri Ducard'}] | (:Film {title:'Batman Begins'}) |
    When executing query
      '''
      MATCH (a) WHERE a.name IN ['Dennis Quaid', 'Christopher Nolan'] WITH a MATCH (b) WHERE b.name IN ['London'] RETURN a, b;
      '''
    Then the result should be, in any order
      | a                                                   | b                       |
      | (:Person {name:'Dennis Quaid',birthyear:1954})      | (:City {name:'London'}) |
      | (:Person {name:'Christopher Nolan',birthyear:1970}) | (:City {name:'London'}) |
    When executing query
      '''
      MATCH (a) WHERE a.name IN ['Dennis Quaid', 'Christopher Nolan'] WITH a MATCH (b) WHERE b.name IN ['London', 'Beijing', 'Houston'] RETURN a, b;
      '''
    Then the result should be, in any order
      | a                                                   | b                        |
      | (:Person {name:'Dennis Quaid',birthyear:1954})      | (:City {name:'London'})  |
      | (:Person {name:'Dennis Quaid',birthyear:1954})      | (:City {name:'Houston'}) |
      | (:Person {name:'Christopher Nolan',birthyear:1970}) | (:City {name:'London'})  |
      | (:Person {name:'Christopher Nolan',birthyear:1970}) | (:City {name:'Houston'}) |
    When executing query
      '''
      MATCH (n:Person) WHERE n.name = 'Vanessa Redgrave' OR NOT n.name <> 'Dennis Quaid' RETURN n.name;
      '''
    Then the result should be, in any order
      | n.name           |
      | 'Vanessa Redgrave' |
      | 'Dennis Quaid'     |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[:BORN_IN|ACTED_IN]->(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                         |
      | (:Film {title:'Camelot'}) |
      | (:City {name:'London'})   |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})<-[:MARRIED|HAS_CHILD]-(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                |
      | (:Person {name:'Roy Redgrave',birthyear:1873})   |
      | (:Person {name:'Rachel Kempson',birthyear:1910}) |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[:BORN_IN|HAS_CHILD]-(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:City {name:'London'})                              |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})--(m:Person) WHERE m.birthyear > n.birthyear RETURN m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})--(m:Person) WHERE n.birthyear > m.birthyear RETURN m;
      '''
    Then the result should be, in any order
      | m                                                  |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
    When executing query
      '''
      MATCH (n:Person {name:'Natasha Richardson'})-[:HAS_CHILD]->(m) RETURN m;
      '''
    Then the result should be, in any order
      | m |
    When executing query
      '''
      MATCH (n:Person {name:'Natasha Richardson'})<-[:HAS_CHILD]-(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
    When executing query
      '''
      MATCH (n:Person {name:'Natasha Richardson'})-[:HAS_CHILD]-(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
    When executing query
      '''
      match (a)-->(b)-->(c)<--(d) where not ((not (a.birthyear>d.birthyear and b.birthyear<c.birthyear)) and (not a.birthyear>b.birthyear)) return d;
      '''
    Then the result should be, in any order
      | d                                                   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})    |
      | (:Person {name:'Rachel Kempson',birthyear:1910})    |
      | (:Person {name:'John Williams',birthyear:1932})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})  |
      | (:Person {name:'Christopher Nolan',birthyear:1970}) |
    When executing query
      '''
      MATCH (n:Person{name:'Vanessa Redgrave'}),(m:Person{name: 'Michael Redgrave'}) WHERE n.birthyear > 1960 and m.birthyear < 2000 RETURN n.name LIMIT 1;
      '''
    Then the result should be, in any order
      | n.name |
    When executing query
      '''
      MATCH (n:Person) RETURN count(n);
      '''
    Then the result should be, in any order
      | count(n) |
      | 13       |
    When executing query
      '''
      MATCH (n:Person) WHERE n.birthyear > 1900 AND n.birthyear < 2000 RETURN count(n);
      '''
    Then the result should be, in any order
      | count(n) |
      | 12       |
    When executing query
      '''
      MATCH (n:Person) RETURN n.birthyear, count(n);
      '''
    Then the result should be, in any order
      | n.birthyear | count(n) |
      | 1970        | 1        |
      | 1986        | 1        |
      | 1932        | 1        |
      | 1965        | 1        |
      | 1954        | 1        |
      | 1930        | 1        |
      | 1952        | 1        |
      | 1939        | 1        |
      | 1873        | 1        |
      | 1937        | 1        |
      | 1963        | 1        |
      | 1908        | 1        |
      | 1910        | 1        |
    When executing query
      '''
      MATCH (f:Film)<-[:ACTED_IN]-(p:Person)-[:BORN_IN]->(c:City) RETURN c.name, count(f) AS sum ORDER BY sum DESC;
      '''
    Then the result should be, in any order
      | c.name   | sum |
      | 'London'   | 2   |
      | 'New York' | 1   |
      | 'Houston'  | 1   |
    When executing query
      '''
      MATCH p=(n1)-[r1]->(n2)-[r2]->(m:Person) return count(p);
      '''
    Then the result should be, in any order
      | count(p) |
      | 16       |
    When executing query
      '''
      MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City),p2=(n3)-[r3]->(m2:Film) return count(p1);
      '''
    Then the result should be, in any order
      | count(p1) |
      | 44        |
    When executing query
      '''
      MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City) with count(p1) as cp match p1=(n1)-[r1]->(m1:Film) return count(p1);
      '''
    Then the result should be, in any order
      | count(p1) |
      | 11        |