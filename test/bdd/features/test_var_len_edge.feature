Feature: test var len edge
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]->(n)-[:ACTED_IN]->(m) RETURN n,m;
      '''
    Then the result should be, in any order
      | n                                                    | m                                   |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Film {title:'Goodbye Mr. Chips'}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Film {title:'Camelot'})           |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Film {title:'The Parent Trap'})   |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..5]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..2]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2..5]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2..]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..2]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..3]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                  |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[*..]->(n) RETURN DISTINCT n,n.name,n.title;
      '''
    Then the result should be, in any order
      | n                                                    | n.name               | n.title           |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | 'Natasha Richardson' | null              |
      | (:City {name:'London'})                              | 'London'             | null              |
      | (:Film {title:'Camelot'})                            | null                 | 'Camelot'         |
      | (:Person {name:'Liam Neeson',birthyear:1952})        | 'Liam Neeson'        | null              |
      | (:Film {title:'The Parent Trap'})                    | null                 | 'The Parent Trap' |
      | (:Film {title:'Batman Begins'})                      | null                 | 'Batman Begins'   |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[*3]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Film {title:'Batman Begins'})                      |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[*2..]->(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:City {name:'London'})                              |
      | (:Film {title:'The Parent Trap'})                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Film {title:'Batman Begins'})                      |
      | (:City {name:'London'})                              |
      | (:Film {title:'The Parent Trap'})                    |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[]->()-[*0]->(m) RETURN m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:City {name:'London'})                              |
      | (:Film {title:'Camelot'})                            |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[]->()-[*0..1]->(m) RETURN DISTINCT m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:City {name:'London'})                              |
      | (:Film {title:'The Parent Trap'})                    |
      | (:Film {title:'Camelot'})                            |
    When executing query
      '''
      MATCH (mic:Person {name:'Michael Redgrave'})-[]->()-[*0..1]->(m) RETURN DISTINCT m;
      '''
    Then the result should be, in any order
      | m                                                    |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:City {name:'London'})                              |
      | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Film {title:'Goodbye Mr. Chips'})                  |
    When executing query
      '''
      MATCH (jem:Person {name:'Jemma Redgrave'})<-[:HAS_CHILD*..]-(a) RETURN a;
      '''
    Then the result should be, in any order
      | a                                                  |
      | (:Person {name:'Corin Redgrave',birthyear:1939})   |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     |
    When executing query
      '''
      MATCH (jem:Person {name:'Jemma Redgrave'})<-[:HAS_CHILD*..]-(a)-[:ACTED_IN*..]->(m) RETURN a,m;
      '''
    Then the result should be, in any order
      | a                                                  | m                                   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Goodbye Mr. Chips'}) |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD|MARRIED*..]->(n) RETURN DISTINCT n.name;
      '''
    Then the result should be, in any order
      | n.name             |
      | 'Michael Redgrave'   |
      | 'Vanessa Redgrave'   |
      | 'Corin Redgrave'     |
      | 'Rachel Kempson'     |
      | 'Natasha Richardson' |
      | 'Jemma Redgrave'     |
      | 'Liam Neeson'        |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD|MARRIED*1..2]->(n) RETURN DISTINCT n.name;
      '''
    Then the result should be, in any order
      | n.name           |
      | 'Michael Redgrave' |
      | 'Vanessa Redgrave' |
      | 'Corin Redgrave'   |
      | 'Rachel Kempson'   |
    When executing query
      '''
      MATCH (liam:Person {name:'Liam Neeson'})<-[:HAS_CHILD|MARRIED*1..3]-(a) RETURN DISTINCT a.name;
      '''
    Then the result should be, in any order
      | a.name             |
      | 'Natasha Richardson' |
      | 'Vanessa Redgrave'   |
      | 'Liam Neeson'        |
      | 'Rachel Kempson'     |
      | 'Michael Redgrave'   |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]-(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
    When executing query
      '''
      MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]-(n) RETURN DISTINCT n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
    When executing query
      '''
      MATCH (jem:Person {name:'Jemma Redgrave'})-[:HAS_CHILD*..]-(a) RETURN a;
      '''
    Then the result should be, in any order
      | a                                                    |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
    When executing query
      '''
      MATCH (jem:Person {name:'Jemma Redgrave'})-[:HAS_CHILD*..]-(a) RETURN DISTINCT a;
      '''
    Then the result should be, in any order
      | a                                                    |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*..]-(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*..]-(n) RETURN DISTINCT n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*1..2]-(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*2]-(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                |
      | (:Person {name:'Corin Redgrave',birthyear:1939}) |
      | (:Person {name:'Corin Redgrave',birthyear:1939}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})   |
    When executing query
      '''
      MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*2..]-(n) RETURN n;
      '''
    Then the result should be, in any order
      | n                                                    |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
    When executing query
      '''
      MATCH (n:Person)-[:BORN_IN*0..]->(m) RETURN n.name,m.name;
      '''
    Then the result should be, in any order
      | n.name             | m.name             |
      | 'Rachel Kempson'     | 'Rachel Kempson'     |
      | 'Michael Redgrave'   | 'Michael Redgrave'   |
      | 'Vanessa Redgrave'   | 'Vanessa Redgrave'   |
      | 'Vanessa Redgrave'   | 'London'             |
      | 'Corin Redgrave'     | 'Corin Redgrave'     |
      | 'Liam Neeson'        | 'Liam Neeson'        |
      | 'Natasha Richardson' | 'Natasha Richardson' |
      | 'Natasha Richardson' | 'London'             |
      | 'Richard Harris'     | 'Richard Harris'     |
      | 'Dennis Quaid'       | 'Dennis Quaid'       |
      | 'Dennis Quaid'       | 'Houston'            |
      | 'Lindsay Lohan'      | 'Lindsay Lohan'      |
      | 'Lindsay Lohan'      | 'New York'           |
      | 'Jemma Redgrave'     | 'Jemma Redgrave'     |
      | 'Roy Redgrave'       | 'Roy Redgrave'       |
      | 'John Williams'      | 'John Williams'      |
      | 'John Williams'      | 'New York'           |
      | 'Christopher Nolan'  | 'Christopher Nolan'  |
      | 'Christopher Nolan'  | 'London'             |
    When executing query
      '''
      MATCH (n:Person)-[:BORN_IN*0..]->(m:City) RETURN n.name,m.name;
      '''
    Then the result should be, in any order
      | n.name             | m.name   |
      | 'Vanessa Redgrave'   | 'London'   |
      | 'Natasha Richardson' | 'London'   |
      | 'Dennis Quaid'       | 'Houston'  |
      | 'Lindsay Lohan'      | 'New York' |
      | 'John Williams'      | 'New York' |
      | 'Christopher Nolan'  | 'London'   |
    When executing query
      '''
      MATCH (n:Person)-[:BORN_IN*0..]->(m:Person) RETURN n.name,m.name;
      '''
    Then the result should be, in any order
      | n.name               | m.name             |
      | 'Rachel Kempson'     | 'Rachel Kempson'     |
      | 'Michael Redgrave'   | 'Michael Redgrave'   |
      | 'Vanessa Redgrave'   | 'Vanessa Redgrave'   |
      | 'Corin Redgrave'     | 'Corin Redgrave'     |
      | 'Liam Neeson'        | 'Liam Neeson'        |
      | 'Natasha Richardson' | 'Natasha Richardson' |
      | 'Richard Harris'     | 'Richard Harris'     |
      | 'Dennis Quaid'       | 'Dennis Quaid'       |
      | 'Lindsay Lohan'      | 'Lindsay Lohan'      |
      | 'Jemma Redgrave'     | 'Jemma Redgrave'     |
      | 'Roy Redgrave'       | 'Roy Redgrave'       |
      | 'John Williams'      | 'John Williams'      |
      | 'Christopher Nolan'  | 'Christopher Nolan'  |
    When executing query
      '''
      MATCH (n:Film)<-[:ACTED_IN*0..]-(m) RETURN n.title,n,m;
      '''
    Then the result should be, in any order
      | n.title                                 | n                                                       | m                                                       |
      | 'Goodbye Mr. Chips'                     | (:Film {title:'Goodbye Mr. Chips'})                     | (:Film {title:'Goodbye Mr. Chips'})                     |
      | 'Goodbye Mr. Chips'                     | (:Film {title:'Goodbye Mr. Chips'})                     | (:Person {name:'Michael Redgrave',birthyear:1908})      |
      | 'Batman Begins'                         | (:Film {title:'Batman Begins'})                         | (:Film {title:'Batman Begins'})                         |
      | 'Batman Begins'                         | (:Film {title:'Batman Begins'})                         | (:Person {name:'Liam Neeson',birthyear:1952})           |
      | 'Harry Potter and the Sorcerer's Stone' | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) |
      | 'Harry Potter and the Sorcerer's Stone' | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:Person {name:'Richard Harris',birthyear:1930})        |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Film {title:'The Parent Trap'})                       |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Natasha Richardson',birthyear:1963})    |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Dennis Quaid',birthyear:1954})          |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Lindsay Lohan',birthyear:1986})         |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Film {title:'Camelot'})                               |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Person {name:'Vanessa Redgrave',birthyear:1937})      |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Person {name:'Richard Harris',birthyear:1930})        |
    When executing query
      '''
      MATCH (n:Film)<-[:ACTED_IN*0..]-(m:Person) RETURN n.title,n,m;
      '''
    Then the result should be, in any order
      | n.title                                 | n                                                       | m                                                    |
      | 'Goodbye Mr. Chips'                     | (:Film {title:'Goodbye Mr. Chips'})                     | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | 'Batman Begins'                         | (:Film {title:'Batman Begins'})                         | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | 'Harry Potter and the Sorcerer's Stone' | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:Person {name:'Richard Harris',birthyear:1930})     |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Dennis Quaid',birthyear:1954})       |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Person {name:'Lindsay Lohan',birthyear:1986})      |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Person {name:'Richard Harris',birthyear:1930})     |
    When executing query
      '''
      MATCH (n:Film)<-[:ACTED_IN*0..]-(m:Film) RETURN n.title,n,m;
      '''
    Then the result should be, in any order
      | n.title                                 | n                                                       | m                                                       |
      | 'Goodbye Mr. Chips'                     | (:Film {title:'Goodbye Mr. Chips'})                     | (:Film {title:'Goodbye Mr. Chips'})                     |
      | 'Batman Begins'                         | (:Film {title:'Batman Begins'})                         | (:Film {title:'Batman Begins'})                         |
      | 'Harry Potter and the Sorcerer's Stone' | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) |
      | 'The Parent Trap'                       | (:Film {title:'The Parent Trap'})                       | (:Film {title:'The Parent Trap'})                       |
      | 'Camelot'                               | (:Film {title:'Camelot'})                               | (:Film {title:'Camelot'})                               |
    When executing query
      '''
      MATCH (n:Film)<-[:ACTED_IN*0..]-(m:City) RETURN n.title,n,m;
      '''
    Then the result should be, in any order
      | n.title | n | m |