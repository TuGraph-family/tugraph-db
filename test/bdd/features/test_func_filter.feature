Feature: test func filter
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n) WHERE id(n) = 6 RETURN n.name;
      '''
    Then the result should be, in any order
      | n.name             |
      | 'Natasha Richardson' |
    When executing query
      '''
      MATCH (n) WHERE id(n) <> 6 RETURN n;
      '''
    Then the result should be, in any order
      | n                                                       |
      | (:Person {name:'Rachel Kempson',birthyear:1910})        |
      | (:Person {name:'Michael Redgrave',birthyear:1908})      |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937})      |
      | (:Person {name:'Corin Redgrave',birthyear:1939})        |
      | (:Person {name:'Liam Neeson',birthyear:1952})           |
      | (:Person {name:'Richard Harris',birthyear:1930})        |
      | (:Person {name:'Dennis Quaid',birthyear:1954})          |
      | (:Person {name:'Lindsay Lohan',birthyear:1986})         |
      | (:Person {name:'Jemma Redgrave',birthyear:1965})        |
      | (:Person {name:'Roy Redgrave',birthyear:1873})          |
      | (:Person {name:'John Williams',birthyear:1932})         |
      | (:Person {name:'Christopher Nolan',birthyear:1970})     |
      | (:City {name:'New York'})                               |
      | (:City {name:'London'})                                 |
      | (:City {name:'Houston'})                                |
      | (:Film {title:'Goodbye Mr. Chips'})                     |
      | (:Film {title:'Batman Begins'})                         |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) |
      | (:Film {title:'The Parent Trap'})                       |
      | (:Film {title:'Camelot'})                               |
    When executing query
      '''
      MATCH ()-[r]->() WHERE type(r) = 'ACTED_IN' RETURN r,type(r);
      '''
    Then the result should be, in any order
      | r                                              | type(r)  |
      | [:ACTED_IN {charactername:'The Headmaster'}]   | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Guenevere'}]        | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Henri Ducard'}]     | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Liz James'}]        | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Albus Dumbledore'}] | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'King Arthur'}]      | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Nick Parker'}]      | 'ACTED_IN' |
      | [:ACTED_IN {charactername:'Halle/Annie'}]      | 'ACTED_IN' |
    When executing query
      '''
      MATCH ()-[r]->() WHERE type(4) = 'ACTED_IN' RETURN r,type(r);
      '''
    Then an Error should be raised
    When executing query
      '''
      MATCH (a)-->(b)-->(c)<--(d) WHERE id(b) <> id(d) AND id(a) > id(d) AND id(b) < id(c) RETURN a,b,c,d;
      '''
    Then the result should be, in any order
      | a                                                    | b                                                    | c                                                    | d                                                  |
      | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:City {name:'London'})                              | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Liam Neeson',birthyear:1952})        | (:Person {name:'Natasha Richardson',birthyear:1963}) | (:Person {name:'Vanessa Redgrave',birthyear:1937}) |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Vanessa Redgrave',birthyear:1937})   | (:Person {name:'Rachel Kempson',birthyear:1910})   |
      | (:Person {name:'Roy Redgrave',birthyear:1873})       | (:Person {name:'Michael Redgrave',birthyear:1908})   | (:Person {name:'Corin Redgrave',birthyear:1939})     | (:Person {name:'Rachel Kempson',birthyear:1910})   |
