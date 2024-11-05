Feature: test with
  Scenario: case01
    Given yago graph
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n as n1 match (n {name:'John Williams'}) return n,n1;
      '''
    Then the result should be, in any order
      | n                                               | n1                                            |
      | (:Person {name:'John Williams',birthyear:1932}) | (:Person {name:'Liam Neeson',birthyear:1952}) |
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n as n1 match (n {name:'Dennis Quaid'}) with n as n2, n1 match (n {name:'John Williams'}) return n,n2,n1;
      '''
    Then the result should be, in any order
      | n                                               | n2                                             | n1                                            |
      | (:Person {name:'John Williams',birthyear:1932}) | (:Person {name:'Dennis Quaid',birthyear:1954}) | (:Person {name:'Liam Neeson',birthyear:1952}) |
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n as n1 match (n {name:'Dennis Quaid'}) with n as n2, n1.name as n1name match (n {name:'John Williams'}) return n,n2,n1name;
      '''
    Then the result should be, in any order
      | n                                               | n2                                             | n1name      |
      | (:Person {name:'John Williams',birthyear:1932}) | (:Person {name:'Dennis Quaid',birthyear:1954}) | 'Liam Neeson' |
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n return n;
      '''
    Then the result should be, in any order
      | n                                             |
      | (:Person {name:'Liam Neeson',birthyear:1952}) |
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n match (n)-->(m) return n,m;
      '''
    Then the result should be, in any order
      | n                                             | m                                                    |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Film {title:'Batman Begins'})                      |
    When executing query
      '''
      match (n {name:'London'}) with n optional match (n)-->(m) return n,m;
      '''
    Then the result should be, in any order
      | n                       | m    |
      | (:City {name:'London'}) | null |
    When executing query
      '''
      match (a {name:'Liam Neeson'})-[r]->(b) with b match (b)-[]->(c) return c;
      '''
    Then the result should be, in any order
      | c                                             |
      | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:City {name:'London'})                       |
      | (:Film {title:'The Parent Trap'})             |
    When executing query
      '''
      match (a {name:'Liam Neeson'}),(b {name:'London'}) with a, b match (c:Film) return a,b,c;
      '''
    Then the result should be, in any order
      | a                                             | b                       | c                                                       |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:City {name:'London'}) | (:Film {title:'Goodbye Mr. Chips'})                     |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:City {name:'London'}) | (:Film {title:'Batman Begins'})                         |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:City {name:'London'}) | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:City {name:'London'}) | (:Film {title:'The Parent Trap'})                       |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:City {name:'London'}) | (:Film {title:'Camelot'})                               |
    When executing query
      '''
      match (n {name:'Liam Neeson'}) with n match (n) return n.name;
      '''
    Then the result should be, in any order
      | n.name      |
      | 'Liam Neeson' |
    When executing query
      '''
      match (a {name:'Liam Neeson'}), (b {name:'London'}) with a, b match (a), (b) return a.name, b.name;
      '''
    Then the result should be, in any order
      | a.name      | b.name |
      | 'Liam Neeson' | 'London' |
    When executing query
      '''
      MATCH (a {name:'Liam Neeson'})-[r]->(b) RETURN a,count(b) AS out_num;
      '''
    Then the result should be, in any order
      | a                                             | out_num |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | 2       |
    When executing query
      '''
      MATCH (a {name:'Liam Neeson'})-[r]->(b) WITH a,count(b) AS out_num MATCH (a)<-[]-(c) RETURN count(c) AS in_num,out_num;
      '''
    Then the result should be, in any order
      | in_num | out_num |
      | 1      | 2       |
    When executing query
      '''
      match (a {name:'Liam Neeson'})-[r]->(b) with a,b match (b)-[]->(c) return a,b,c;
      '''
    Then the result should be, in any order
      | a                                             | b                                                    | c                                             |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:City {name:'London'})                       |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Film {title:'The Parent Trap'})             |
    When executing query
      '''
      match (n {name:'Liam Neeson'}),(m {name:'Natasha Richardson'}),(n)-[r]->(m) return r,type(r);
      '''
    Then the result should be, in any order
      | r             | type(r) |
      | [:MARRIED] | 'MARRIED' |
    When executing query
      '''
      match (n {name:'Liam Neeson'}),(m {name:'Natasha Richardson'}) with n,m match (n)-[r]->(m) return r,type(r);
      '''
    Then the result should be, in any order
      | r             | type(r) |
      | [:MARRIED] | 'MARRIED' |
    When executing query
      '''
      match (n {name:'Liam Neeson'}),(m {name:'Liam Neeson'}) with n,m optional match (n)-[r]->(m) return r,type(r);
      '''
    Then the result should be, in any order
      | r    | type(r) |
      | null | null    |
    When executing query
      '''
      match (n {name:'Liam Neeson'})-[r]->(m) with r return r,type(r);
      '''
    Then the result should be, in any order
      | r                                          | type(r)  |
      | [:MARRIED]                              | 'MARRIED'  |
      | [:ACTED_IN {charactername:'Henri Ducard'}] | 'ACTED_IN' |
    When executing query
      '''
      match (n:City) with count (n) as num_city match (n:Film) return count(n) as num_film, num_city;
      '''
    Then the result should be, in any order
      | num_film | num_city |
      | 5        | 3        |
    When executing query
      '''
      match (n:Person {name:'Vanessa Redgrave'})-->(m) with m as m1 match (n:Person {name:'Vanessa Redgrave'})<--(m) return m as m2, m1;
      '''
    Then the result should be, in any order
      | m2                                                 | m1                                                   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:City {name:'London'})                              |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:City {name:'London'})                              |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Camelot'})                            |
    When executing query
      '''
      match (n:Person {name:'Vanessa Redgrave'})-->(m) with count(m) as c1 match (n:Person {name:'Vanessa Redgrave'})<--(m) return count(m) as c2, c1;
      '''
    Then the result should be, in any order
      | c2 | c1 |
      | 2  | 3  |
    When executing query
      '''
      match (n:Person {name:'Vanessa Redgrave'})-->(m) with count(m) as cm1 match (n:Person {name:'Vanessa Redgrave'})<--(m) with count(m) as cm2, cm1 match (n:Person {name:'Natasha Richardson'})-->(m) return count(m) as cm3, cm2, cm1;
      '''
    Then the result should be, in any order
      | cm3 | cm2 | cm1 |
      | 3   | 2   | 3   |
    When executing query
      '''
      match (n:Person {name:'Michael Redgrave'})-->(m:Person) where m.birthyear > 1938 with count(m) as p38 match (n:Person {name:'Michael Redgrave'})-->(m:Person) where m.birthyear > 1908 return count(m) as p08,p38 /* 3,1 */;
      '''
    Then the result should be, in any order
      | p08 | p38 |
      | 3   | 1   |
    When executing query
      '''
      WITH 2020 AS x WHERE x > 2020 RETURN x;
      '''
    Then the result should be, in any order
      | x |
    When executing query
      '''
      MATCH (n:City) WITH 2020 AS x, n.name AS y ORDER BY y WHERE x = 2020 RETURN x,y;
      '''
    Then the result should be, in any order
      | x    | y        |
      | 2020 | 'Houston'  |
      | 2020 | 'London'   |
      | 2020 | 'New York' |
    When executing query
      '''
      MATCH (n) WITH n WHERE n.name = 'Liam Neeson' MATCH (m {name:'John Williams'}) RETURN n,m;
      '''
    Then the result should be, in any order
      | n                                             | m                                               |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'John Williams',birthyear:1932}) |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})-->(m:Person) WHERE m.birthyear > 1908 WITH count(m) AS p08 RETURN p08 /* 3 */;
      '''
    Then the result should be, in any order
      | p08 |
      | 3   |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(m) WITH m, count(*) AS edge_num WHERE edge_num > 1.0 RETURN m.name,edge_num;
      '''
    Then the result should be, in any order
      | m.name         | edge_num |
      | 'Rachel Kempson' | 2        |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(m) WITH n, m, count(*) AS edge_num WHERE edge_num > 1.0 OR n.birthyear > 1900 RETURN m.name,edge_num;
      '''
    Then the result should be, in any order
      | m.name         | edge_num |
      | 'Rachel Kempson' | 1        |
      | 'Rachel Kempson' | 1        |
      | 'Rachel Kempson' | 2        |
      | 'Rachel Kempson' | 1        |
      | 'Rachel Kempson' | 1        |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(m) WITH m, count(*) AS edge_num WHERE edge_num > 1.0 AND m.birthyear > 1900 RETURN m.name,edge_num;
      '''
    Then the result should be, in any order
      | m.name         | edge_num |
      | 'Rachel Kempson' | 2        |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(nbr)-->() WITH nbr, count(*) AS foaf WHERE foaf > 1.0 RETURN nbr.name,foaf;
      '''
    Then the result should be, in any order
      | nbr.name       | foaf |
      | 'Rachel Kempson' | 5    |
      | 'Rachel Kempson' | 3    |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'}) WHERE n.birthyear > 1900 AND n.birthyear < 2000 RETURN n.name;
      '''
    Then the result should be, in any order
      | n.name           |
      | 'Michael Redgrave' |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(m) WITH m, count(*) AS edge_num WHERE toInteger(edge_num) > 1 RETURN m.name,edge_num;
      '''
    Then the result should be, in any order
      | m.name         | edge_num |
      | 'Rachel Kempson' | 2        |
    When executing query
      '''
      MATCH (n:Person {name:'Michael Redgrave'})--(nbr)-->() WITH nbr, count(*) AS foaf WHERE toInteger(foaf) > 1 RETURN nbr.name,foaf;
      '''
    Then the result should be, in any order
      | nbr.name       | foaf |
      | 'Rachel Kempson' | 5    |
      | 'Rachel Kempson' | 3    |
    When executing query
      '''
      MATCH (a:City) WITH a MATCH (b:Person {name:'Liam Neeson'}) RETURN a,b;
      '''
    Then the result should be, in any order
      | a                         | b                                             |
      | (:City {name:'New York'}) | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:City {name:'London'})   | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:City {name:'Houston'})  | (:Person {name:'Liam Neeson',birthyear:1952}) |
    When executing query
      '''
      WITH 'Vanessa Redgrave' AS varName MATCH (n:Film) RETURN n,varName;
      '''
    Then the result should be, in any order
      | n                                                       | varName          |
      | (:Film {title:'Goodbye Mr. Chips'})                     | 'Vanessa Redgrave' |
      | (:Film {title:'Batman Begins'})                         | 'Vanessa Redgrave' |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | 'Vanessa Redgrave' |
      | (:Film {title:'The Parent Trap'})                       | 'Vanessa Redgrave' |
      | (:Film {title:'Camelot'})                               | 'Vanessa Redgrave' |
    When executing query
      '''
      WITH 'Vanessa Redgrave' AS varName MATCH (n {name:varName}) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
    When executing query
      '''
      MATCH (n {birthyear:1952}) WITH n,n.name AS varName MATCH (m {name:varName}) RETURN n,m;
      '''
    Then the result should be, in any order
      | n                                             | m                                             |
      | (:Person {name:'Liam Neeson',birthyear:1952}) | (:Person {name:'Liam Neeson',birthyear:1952}) |
    When executing query
      '''
      WITH 1 AS a MATCH (n:City) RETURN DISTINCT a,n;
      '''
    Then the result should be, in any order
      | a | n                         |
      | 1 | (:City {name:'New York'}) |
      | 1 | (:City {name:'London'})   |
      | 1 | (:City {name:'Houston'})  |
    When executing query
      '''
      MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m MATCH (m)-[:ACTED_IN]->(film) RETURN m.name,film;
      '''
    Then the result should be, in any order
      | m.name           | film                                |
      | 'Vanessa Redgrave' | (:Film {title:'Camelot'})           |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'}) |
    When executing query
      '''
      MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m OPTIONAL MATCH (m)-[:ACTED_IN]->(film) RETURN m.name,film;
      '''
    Then the result should be, in any order
      | m.name           | film                                |
      | 'Vanessa Redgrave' | (:Film {title:'Camelot'})           |
      | 'Corin Redgrave'   | (:Film {title:'Camelot'})           |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'}) |
    When executing query
      '''
      MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m OPTIONAL MATCH (m)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(coactor) RETURN m.name,film,coactor;
      '''
    Then the result should be, in any order
      | m.name           | film                                | coactor                                          |
      | 'Vanessa Redgrave' | (:Film {title:'Camelot'})           | (:Person {name:'Richard Harris',birthyear:1930}) |
      | 'Corin Redgrave'   | (:Film {title:'Camelot'})           | (:Person {name:'Richard Harris',birthyear:1930}) |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'Richard Harris',birthyear:1930}) |
    When executing query
      '''
      MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m OPTIONAL MATCH (m)-[:ACTED_IN]->(film) WITH m,film RETURN m.name,film;
      '''
    Then the result should be, in any order
      | m.name           | film                                |
      | 'Vanessa Redgrave' | (:Film {title:'Camelot'})           |
      | 'Corin Redgrave'   | (:Film {title:'Camelot'})           |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'}) |
    When executing query
      '''
      MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m OPTIONAL MATCH (m)-[:ACTED_IN]->(film) WITH m,film OPTIONAL MATCH (film)<-[:WROTE_MUSIC_FOR]-(musician) RETURN m.name,film,musician;
      '''
    Then the result should be, in any order
      | m.name           | film                                | musician                                        |
      | 'Vanessa Redgrave' | (:Film {title:'Camelot'})           | null                                            |
      | 'Corin Redgrave'   | (:Film {title:'Camelot'})           | null                                            |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'John Williams',birthyear:1932}) |
    When executing query
      '''
      match (n:Person) where n.name='Michael Redgrave' with n.birthyear as nb match (p)-[:HAS_CHILD]->(c) where p.birthyear=nb return c.name;
      '''
    Then the result should be, in any order
      | c.name           |
      | 'Vanessa Redgrave' |
      | 'Corin Redgrave'   |
    When executing query
      '''
      match (n:Person) where n.name='Roy Redgrave' or n.name='Michael Redgrave' with collect(id(n)) as cn match (p:Person) where id(p) in cn return p.name;
      '''
    Then the result should be, in any order
      | p.name           |
      | 'Michael Redgrave' |
      | 'Roy Redgrave'     |
    When executing query
      '''
      match (n:Person) where n.name='Roy Redgrave' or n.name='Michael Redgrave' with n, collect(id(n)) as cn match (p:Person) where id(p) in cn return p.name;
      '''
    Then the result should be, in any order
      | p.name           |
      | 'Roy Redgrave'     |
      | 'Michael Redgrave' |
    When executing query
      '''
      match (c:Person)-[:HAS_CHILD]->(f:Person) where c.name='Roy Redgrave' with c, f match (m:Person)-[:ACTED_IN]->(film:Film)<-[:WROTE_MUSIC_FOR]-(p:Person) where m.name=f.name return c.name, p.name;
      '''
    Then the result should be, in any order
      | c.name       | p.name        |
      | 'Roy Redgrave' | 'John Williams' |