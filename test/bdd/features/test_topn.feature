Feature: test order by
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (a:Person {name:'Lindsay Lohan'}), (b:Film {title:'The Parent Trap'}) CREATE (a)-[r:DIRECTED]->(b);
      '''
    Then the result should be, in any order
      | <SUMMARY>                            |
      | 'created 0 vertices, created 1 edges.' |
    When executing query
      '''
      match (v1:Film) return v1.title order by v1.title limit 3;
      '''
    Then the result should be, in order
      | v1.title          |
      | 'Batman Begins     '|
      | 'Camelot           '|
      | 'Goodbye Mr. Chips '|
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,v2.name as cnt order by cnt desc limit 3;
      '''
    Then the result should be, in order
      | v1.title                              | cnt              |
      | 'Camelot'                               | 'Vanessa Redgrave' |
      | 'Camelot'                               | 'Richard Harris'   |
      | 'Harry Potter and the Sorcerer's Stone' | 'Richard Harris'   |
    When executing query
      '''
      match (:Person {name:'Vanessa Redgrave'})<-[:HAS_CHILD]-(p)-[:ACTED_IN*0..]->(m) return p.name,m order by p.name limit 3;
      '''
    Then the result should be, in order
      | p.name           | m                                                  |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'})                |
      | 'Michael Redgrave' | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | 'Rachel Kempson'   | (:Person {name:'Rachel Kempson',birthyear:1910})   |
    When executing query
      '''
      MATCH (n) RETURN n.name AS name ORDER BY name LIMIT 10;
      '''
    Then the result should be, in order
      | name              |
      | null              |
      | null              |
      | null              |
      | null              |
      | null              |
      | 'Christopher Nolan' |
      | 'Corin Redgrave'    |
      | 'Dennis Quaid'      |
      | 'Houston'           |
      | 'Jemma Redgrave'    |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.birthyear, m.name ORDER BY m.name LIMIT 5;
      '''
    Then the result should be, in order
      | m.birthyear | m.name            |
      | 1970        | 'Christopher Nolan' |
      | 1939        | 'Corin Redgrave'    |
      | 1939        | 'Corin Redgrave'    |
      | 1952        | 'Liam Neeson'       |
      | 1952        | 'Liam Neeson'       |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.birthyear, m.name ORDER BY m.birthyear desc LIMIT 5;
      '''
    Then the result should be, in order
      | m.birthyear | m.name             |
      | 1970        | 'Christopher Nolan'  |
      | 1963        | 'Natasha Richardson' |
      | 1952        | 'Liam Neeson'        |
      | 1952        | 'Liam Neeson'        |
      | 1939        | 'Corin Redgrave'     |