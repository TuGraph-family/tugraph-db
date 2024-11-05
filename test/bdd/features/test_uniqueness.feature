Feature: test uniqueness
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-->(n2)-->(n3)-->(n4) RETURN n4.title;
      '''
    Then the result should be, in any order
      | n4.title      |
      | 'Batman Begins' |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<--(n2)<--(n3)<--(n4) RETURN n4;
      '''
    Then the result should be, in any order
      | n4                                                 |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[*3]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                              |
      | (:Film {title:'Batman Begins'}) |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[*..]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Film {title:'Batman Begins'})                      |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:City {name:'London'})                              |
      | (:Film {title:'The Parent Trap'})                    |
      | (:Film {title:'Batman Begins'})                      |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[*2..3]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                            |
      | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:City {name:'London'})                       |
      | (:Film {title:'The Parent Trap'})             |
      | (:Film {title:'Batman Begins'})               |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED*..]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED|ACTED_IN*..]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Film {title:'Batman Begins'})                      |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Film {title:'The Parent Trap'})                    |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Film {title:'Batman Begins'})                      |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED|ACTED_IN*2..]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                            |
      | (:Film {title:'The Parent Trap'})             |
      | (:Person {name:'Liam Neeson',birthyear:1952}) |
      | (:Film {title:'Batman Begins'})               |
    When executing query
      '''
      MATCH (n1:Person {name:'Michael Redgrave'})-[*3]->(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:City {name:'London'})                              |
      | (:Film {title:'The Parent Trap'})                    |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:City {name:'London'})                              |
      | (:Film {title:'Camelot'})                            |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})     |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Corin Redgrave',birthyear:1939})     |
      | (:Film {title:'Goodbye Mr. Chips'})                  |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[*2]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                 |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})      |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[*4]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                 |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[*..]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[*..]-(n2) RETURN DISTINCT n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED*..]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*..]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                   |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})   |
      | (:Person {name:'Liam Neeson',birthyear:1952})        |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})     |
      | (:Person {name:'Michael Redgrave',birthyear:1908})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*2..4]-(n2) RETURN n2;
      '''
    Then the result should be, in any order
      | n2                                                 |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})      |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
    When executing query
      '''
      MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*2..4]-(n2) RETURN DISTINCT n2;
      '''
    Then the result should be, in any order
      | n2                                                 |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Liam Neeson',birthyear:1952})      |
      | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Michael Redgrave',birthyear:1908}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})     |