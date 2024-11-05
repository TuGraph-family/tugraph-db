Feature: test base
  Scenario: case01
    Given yago graph
    When executing query
       '''
       match (n:Person {name:'Vanessa Redgrave',birthyear:1937}),(m:City {name:'London'}) create (n)-[r:BORN_IN {weight:20.21,reg_time:localdatetime('2023-05-01T14:00:00')}]->(m);
       '''
    Then the result should be, in any order
       | <SUMMARY>                            |
       | 'created 0 vertices, created 1 edges.' |
    When executing query
      '''
      MATCH (s)<-[r:BORN_IN]-(d) RETURN s,r,r.weight,d ORDER BY r.weight LIMIT 2;
      '''
    Then the result should be, in any order
      | s                         | r                         | r.weight | d                                              |
      | (:City{name:'Houston'}) | [:BORN_IN {weight:19.11}] | 19.11    | (:Person {name:'Dennis Quaid',birthyear:1954}) |
      | (:City{name:'London'}) | [:BORN_IN {weight:19.93}] | 19.93    | (:Person {name:'Christopher Nolan',birthyear:1970}) |
    When executing query
      '''
      with '123dfd\\fd45a\'bcd' as a return a;
      '''
    Then the result should be, in any order
      | a               |
      | '123dfd\fd45a'bcd' |
    When executing query
      '''
      with "123dfd\\fd45a\'bcd" as a return a;
      '''
    Then the result should be, in any order
      | a                |
      | '123dfd\fd45a'bcd' |
    When executing query
      '''
      optional MATCH (n:City {name:'London'})-[r]->(m) RETURN n.name,r,m;
      '''
    Then the result should be, in any order
      | n.name | r    | m    |
      | 'London' | null | null |
    When executing query
      '''
      MATCH (n:Person)-[r:BORN_IN {reg_time:localdatetime('2023-05-01T14:00:00')}]->(m) RETURN r;
      '''
    Then the result should be, in any order
      | r                                                                  |
      | [:BORN_IN {reg_time:'2023-05-01T14:00:00.000000000',weight:20.21}] |
    When executing query
      '''
      MATCH (n:Person)<-[r:BORN_IN {reg_time:localdatetime('2023-05-01T14:00:00')}]-(m) RETURN r;
      '''
    Then the result should be, in any order
      | r |
    When executing query
      '''
      MATCH (n:Person)-[r:BORN_IN {reg_time:localdatetime('2023-05-01T14:00:00')}]->(m) where r.weight > 20 RETURN r;
      '''
    Then the result should be, in any order
      | r                                                                  |
      | [:BORN_IN {reg_time:'2023-05-01T14:00:00.000000000',weight:20.21}] |
    When executing query
      '''
      MATCH (n:Person)-[r:BORN_IN {reg_time:localdatetime('2023-05-01T14:00:00')}]->(m) where r.weight < 20 RETURN r;
      '''
    Then the result should be, in any order
      | r |
    When executing query
      '''
      MATCH (n:Person)-[r:ACTED_IN {charactername:'Henri Ducard'}]->(m) RETURN r;
      '''
    Then the result should be, in any order
      | r                                          |
      | [:ACTED_IN {charactername:'Henri Ducard'}] |
    When executing query
      '''
      MATCH (n:Person)<-[r:ACTED_IN {charactername:'Henri Ducard'}]-(m) RETURN r;
      '''
    Then the result should be, in any order
      | r |
    When executing query
      '''
      MATCH (p:Person) RETURN p.name ORDER BY p.birthyear DESC LIMIT 5;
      '''
    Then the result should be, in order
      | p.name             |
      | 'Lindsay Lohan      '|
      | 'Christopher Nolan  '|
      | 'Jemma Redgrave     '|
      | 'Natasha Richardson' |
      | 'Dennis Quaid'       |
    When executing query
      '''
      MATCH (p:Person) RETURN p ORDER BY p.birthyear DESC LIMIT 5;
      '''
    Then the result should be, in any order
      | p                                                   |
      | (:Person {name:'Lindsay Lohan',birthyear:1986}) |
      | (:Person {name:'Christopher Nolan',birthyear:1970}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965}) |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Dennis Quaid',birthyear:1954}) |
    When executing query
      '''
      WITH [{a: 'Camelot', b: 'Vanessa Redgrave'},{a: 'Batman Begins', b: 'Liam Neeson'},{a: 'Batman Begins', b: 'Christopher Nolan'}] AS pairs UNWIND pairs AS pair MATCH (n1:Film {title: pair.a})<-[r]-(n2:Person {name: pair.b}) RETURN r;
      '''
    Then the result should be, in any order
      | r                                          |
      | [:ACTED_IN {charactername:'Guenevere'}]    |
      | [:ACTED_IN {charactername:'Henri Ducard'}] |
      | [:DIRECTED]                             |
    When executing query
      '''
      WITH 'Vanessa Redgrave' as names, 'Camelot' as films MATCH (n1:Film {title: films})<-[r]-(n2:Person {name: names}) RETURN r;
      '''
    Then the result should be, in any order
      | r                                       |
      | [:ACTED_IN {charactername:'Guenevere'}] |
    When executing query
      '''
      WITH {a: 'Camelot', b: 'Vanessa Redgrave'} as pair MATCH (n1:Film {title: pair.a})<-[r]-(n2:Person {name: pair.b}) RETURN r;
      '''
    Then the result should be, in any order
      | r                                       |
      | [:ACTED_IN {charactername:'Guenevere'}] |
    When executing query
      '''
      create(n:Person {name:'😎'}) return n;
      '''
    Then the result should be, in any order
      | n                       |
      | (:Person {name:'😎'}) |