Feature: test var len edge
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (p)-[:ACTED_IN]->(x), (p)-[:MARRIED]->(y), (p)-[:HAS_CHILD]->(z) RETURN p,x,y,z;
      '''
    Then the result should be, in any order
      | p                                                  | x                                   | y                                                | z                                                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'Rachel Kempson',birthyear:1910}) | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'Rachel Kempson',birthyear:1910}) | (:Person {name:'Corin Redgrave',birthyear:1939})   |
    When executing query
      '''
      MATCH (x)<-[:ACTED_IN]-(p)-[:MARRIED]->(y), (p)-[:HAS_CHILD]->(z) RETURN p,x,y,z;
      '''
    Then the result should be, in any order
      | p                                                  | x                                   | y                                                | z                                                  |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'Rachel Kempson',birthyear:1910}) | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) | (:Film {title:'Goodbye Mr. Chips'}) | (:Person {name:'Rachel Kempson',birthyear:1910}) | (:Person {name:'Corin Redgrave',birthyear:1939})   |
    When executing query
      '''
      MATCH (n:Film), (m:City) RETURN n, m;
      '''
    Then the result should be, in any order
      | n                                                       | m                         |
      | (:Film {title:'Goodbye Mr. Chips'})                     | (:City {name:'New York'}) |
      | (:Film {title:'Goodbye Mr. Chips'})                     | (:City {name:'London'})   |
      | (:Film {title:'Goodbye Mr. Chips'})                     | (:City {name:'Houston'})  |
      | (:Film {title:'Batman Begins'})                         | (:City {name:'New York'}) |
      | (:Film {title:'Batman Begins'})                         | (:City {name:'London'})   |
      | (:Film {title:'Batman Begins'})                         | (:City {name:'Houston'})  |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:City {name:'New York'}) |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:City {name:'London'})   |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) | (:City {name:'Houston'})  |
      | (:Film {title:'The Parent Trap'})                       | (:City {name:'New York'}) |
      | (:Film {title:'The Parent Trap'})                       | (:City {name:'London'})   |
      | (:Film {title:'The Parent Trap'})                       | (:City {name:'Houston'})  |
      | (:Film {title:'Camelot'})                               | (:City {name:'New York'}) |
      | (:Film {title:'Camelot'})                               | (:City {name:'London'})   |
      | (:Film {title:'Camelot'})                               | (:City {name:'Houston'})  |
    When executing query
      '''
      MATCH (n1:Person {name: 'John Williams'})-[]->(m1:Film), (n2: Person {name: 'Michael Redgrave'})-[]->(m2:Film) WHERE m1.title = m2.title RETURN m1, m2;
      '''
    Then the result should be, in any order
      | m1                                  | m2                                  |
      | (:Film {title:'Goodbye Mr. Chips'}) | (:Film {title:'Goodbye Mr. Chips'}) |