Feature: test find
  Scenario: case01
    Given yago graph
    When executing query
      """
      MATCH (n:Person {name:'Vanessa Redgrave'}) RETURN n;
      """
    Then the result should be, in any order
      | n |
      | (:Person {birthyear: 1937, name: 'VanessaRedgrave'})  |
    When executing query
      """
      MATCH (m:Film {title:'The Parent Trap'}) RETURN m.title,m;
      """
    Then the result should be, in any order
      | m.title | m |
      | 'TheParentTrap'  | (:Film {title: 'TheParentTrap'})|
    When executing query
      """
      MATCH (people:Person) RETURN people.name LIMIT 7;
      """
    Then the result should be, in any order
      | people.name |
      | 'RachelKempson'  |
      | 'MichaelRedgrave'  |
      | 'VanessaRedgrave'  |
      | 'CorinRedgrave'  |
      | 'LiamNeeson'  |
      | 'NatashaRichardson'  |
      | 'RichardHarris'  |
    When executing query
      """
      MATCH (people:Person) RETURN people.name SKIP 7;
      """
    Then the result should be, in any order
      | people.name |
      | 'DennisQuaid'  |
      | 'LindsayLohan'  |
      | 'JemmaRedgrave'  |
      | 'RoyRedgrave'  |
      | 'JohnWilliams'  |
      | 'ChristopherNolan'  |
    When executing query
      """
      MATCH (people:Person) RETURN people.name SKIP 3 LIMIT 4;
      """
    Then the result should be, in any order
      | people.name |
      | 'CorinRedgrave'  |
      | 'LiamNeeson'  |
      | 'NatashaRichardson'  |
      | 'RichardHarris'  |
    When executing query
      """
      MATCH (post60s:Person) WHERE post60s.birthyear >= 1960 AND post60s.birthyear < 1970 RETURN post60s.name;
      """
    Then the result should be, in any order
      | post60s.name |
      | 'NatashaRichardson'  |
      | 'JemmaRedgrave'  |
    When executing query
      """
      MATCH (a:Person) WHERE a.birthyear < 1960 OR a.birthyear >= 1970 RETURN a.name
      """
    Then the result should be, in any order
      | a.name |
      | 'RachelKempson'  |
      | 'MichaelRedgrave'  |
      | 'VanessaRedgrave'  |
      | 'CorinRedgrave'  |
      | 'LiamNeeson'  |
      | 'RichardHarris'  |
      | 'DennisQuaid'  |
      | 'LindsayLohan'  |
      | 'RoyRedgrave'  |
      | 'JohnWilliams'  |
      | 'ChristopherNolan'  |
    When executing query
      """
      MATCH (a:Person) WHERE a.birthyear >= 1960 XOR a.name = 'Jemma Redgrave' RETURN a,a.birthyear;
      """
    Then the result should be, in any order
      | a | a.birthyear |
      | (:Person{birthyear:1963,name:'NatashaRichardson'})  | 1963 |
      | (:Person{birthyear:1986,name:'LindsayLohan'})  | 1986 |
      | (:Person{birthyear:1970,name:'ChristopherNolan'})  | 1970 |
    When executing query
      """
      MATCH (n {name:'Vanessa Redgrave'}) RETURN n;
      """
    Then the result should be, in any order
      | n |
      | (:Person{birthyear:1937,name:'VanessaRedgrave'})  |
    When executing query
      """
      MATCH (n:Person)-[r:BORN_IN]->() WHERE abs(r.weight-20.21)<0.00001 RETURN n,r,r.weight
      """
    Then the result should be, in any order
      | n | r | r.weight |
      | (:Person{birthyear:1937,name:'VanessaRedgrave'})  | [:BORN_IN{weight:20.21}] | 20.21 |