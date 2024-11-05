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
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(v2) as cnt;
      '''
    Then the result should be, in any order
      | v1.title                              | cnt |
      | 'Camelot'                               | 2   |
      | 'The Parent Trap'                       | 4   |
      | 'Harry Potter and the Sorcerer's Stone' | 1   |
      | 'Batman Begins'                         | 2   |
      | 'Goodbye Mr. Chips'                     | 1   |
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt;
      '''
    Then the result should be, in any order
      | v1.title                              | cnt |
      | 'Camelot'                               | 2   |
      | 'The Parent Trap'                       | 3   |
      | 'Harry Potter and the Sorcerer's Stone' | 1   |
      | 'Batman Begins'                         | 2   |
      | 'Goodbye Mr. Chips'                     | 1   |
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt order by cnt;
      '''
    Then the result should be, in order
      | v1.title                              | cnt |
      | 'Harry Potter and the Sorcerer's Stone' | 1   |
      | 'Goodbye Mr. Chips'                     | 1   |
      | 'Camelot'                               | 2   |
      | 'Batman Begins'                         | 2   |
      | 'The Parent Trap'                       | 3   |
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt order by cnt desc;
      '''
    Then the result should be, in order
      | v1.title                              | cnt |
      | 'The Parent Trap'                       | 3   |
      | 'Camelot                               '| 2   |
      | 'Batman Begins                         '| 2   |
      | 'Harry Potter and the Sorcerer's Stone '| 1   |
      | 'Goodbye Mr. Chips                     '| 1   |
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt order by cnt desc,v1.title;
      '''
    Then the result should be, in order
      | v1.title                              | cnt |
      | 'The Parent Trap'                       | 3   |
      | 'Batman Begins'                         | 2   |
      | 'Camelot'                               | 2   |
      | 'Goodbye Mr. Chips'                     | 1   |
      | 'Harry Potter and the Sorcerer's Stone' | 1   |
    When executing query
      '''
      match (v1:Film) return distinct v1.title order by v1.title;
      '''
    Then the result should be, in order
      | v1.title                              |
      | 'Batman Begins'                         |
      | 'Camelot'                               |
      | 'Goodbye Mr. Chips'                     |
      | 'Harry Potter and the Sorcerer's Stone' |
      | 'The Parent Trap'                       |
    When executing query
      '''
      match (v1:Film) return distinct v1.title order by v1.title limit 3 /* Batman, Camelot, Goodbye */;
      '''
    Then the result should be, in order
      | v1.title                              |
      | 'Batman Begins'                         |
      | 'Camelot'                               |
      | 'Goodbye Mr. Chips'                     |
    When executing query
      '''
      match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt order by cnt desc limit 3 /* NOTE: unstable heap sort */;
      '''
    Then the result should be, in order
      | v1.title        | cnt |
      | 'The Parent Trap' | 3   |
      | 'Batman Begins'   | 2   |
      | 'Camelot'         | 2   |
    When executing query
      '''
      match (:Person {name:'Vanessa Redgrave'})<-[:HAS_CHILD]-(p)-[:ACTED_IN*0..]->(m) return p.name,m order by p.name;
      '''
    Then the result should be, in order
      | p.name           | m                                                  |
      | 'Michael Redgrave' | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | 'Michael Redgrave' | (:Film {title:'Goodbye Mr. Chips'})                |
      | 'Rachel Kempson'   | (:Person {name:'Rachel Kempson',birthyear:1910})   |
    When executing query
      '''
      MATCH (n) RETURN n,n.name AS name ORDER BY name;
      '''
    Then the result should be, in order
      | n                                                       | name               |
      | (:Film {title:'Camelot'})                               | null               |
      | (:Film {title:'The Parent Trap'})                       | null               |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | null               |
      | (:Film {title:'Batman Begins'})                         | null               |
      | (:Film {title:'Goodbye Mr. Chips'})                     | null               |
      | (:Person {name:'Christopher Nolan',birthyear:1970})     | 'Christopher Nolan'  |
      | (:Person {name:'Corin Redgrave',birthyear:1939})        | 'Corin Redgrave'     |
      | (:Person {name:'Dennis Quaid',birthyear:1954})          | 'Dennis Quaid'       |
      | (:City {name:'Houston'})                                | 'Houston'            |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})        | 'Jemma Redgrave'     |
      | (:Person {name:'John Williams',birthyear:1932})         | 'John Williams'      |
      | (:Person {name:'Liam Neeson',birthyear:1952})           | 'Liam Neeson'        |
      | (:Person {name:'Lindsay Lohan',birthyear:1986})         | 'Lindsay Lohan'      |
      | (:City {name:'London'})                                 | 'London'             |
      | (:Person {name:'Michael Redgrave',birthyear:1908})      | 'Michael Redgrave'   |
      | (:Person {name:'Natasha Richardson',birthyear:1963})    | 'Natasha Richardson' |
      | (:City {name:'New York'})                               | 'New York       '    |
      | (:Person {name:'Rachel Kempson',birthyear:1910})        | 'Rachel Kempson  '   |
      | (:Person {name:'Richard Harris',birthyear:1930})        | 'Richard Harris  '   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})          | 'Roy Redgrave     '  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})      | 'Vanessa Redgrave'   |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.name;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Liam Neeson'        |
      | 'Liam Neeson'        |
      | 'Natasha Richardson' |
      | 'Christopher Nolan'  |
      | 'Richard Harris'     |
      | 'Corin Redgrave'     |
      | 'Michael Redgrave'   |
      | 'Michael Redgrave'   |
      | 'Corin Redgrave'     |
      | 'Rachel Kempson'     |
      | 'Roy Redgrave'       |
      | 'Rachel Kempson'     |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Liam Neeson'        |
      | 'Natasha Richardson' |
      | 'Christopher Nolan'  |
      | 'Richard Harris'     |
      | 'Corin Redgrave'     |
      | 'Michael Redgrave'   |
      | 'Rachel Kempson'     |
      | 'Roy Redgrave'       |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER BY m.name;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Christopher Nolan'  |
      | 'Corin Redgrave'     |
      | 'Liam Neeson'        |
      | 'Michael Redgrave'   |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
      | 'Richard Harris'     |
      | 'Roy Redgrave'       |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER BY m.name LIMIT 5;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Christopher Nolan'  |
      | 'Corin Redgrave'     |
      | 'Liam Neeson'        |
      | 'Michael Redgrave'   |
      | 'Natasha Richardson' |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER BY m.name SKIP 2;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Liam Neeson'        |
      | 'Michael Redgrave'   |
      | 'Natasha Richardson' |
      | 'Rachel Kempson'     |
      | 'Richard Harris'     |
      | 'Roy Redgrave'       |
    When executing query
      '''
      MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER BY m.name SKIP 2 LIMIT 3;
      '''
    Then the result should be, in order
      | m.name             |
      | 'Liam Neeson'        |
      | 'Michael Redgrave'   |
      | 'Natasha Richardson' |
    When executing query
      '''
      MATCH (v1:Film)<-[r:ACTED_IN|DIRECTED]-(v2:Person) RETURN v1.title AS title, r ORDER BY title LIMIT 5;
      '''
    Then the result should be, in order
      | title             | r                                            |
      | 'Batman Begins'     | [:DIRECTED]                               |
      | 'Batman Begins'     | [:ACTED_IN {charactername:'Henri Ducard'}]   |
      | 'Camelot       '    | [:ACTED_IN {charactername:'King Arthur'}]    |
      | 'Camelot       '    | [:ACTED_IN {charactername:'Guenevere'}]      |
      | 'Goodbye Mr. Chips' | [:ACTED_IN {charactername:'The Headmaster'}] |