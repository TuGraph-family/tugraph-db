Feature: test remove
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (a {name:'Liam Neeson'}) REMOVE a.birthyear RETURN a.name,a.birthyear;
      '''
    Then the result should be, in any order
      | a.name      | a.birthyear |
      | 'Liam Neeson' | null        |
    When executing query
      '''
      MATCH (a {name:'Liam Neeson'}) REMOVE a.name RETURN a.name,a.birthyear;
      '''
    Then the result should be, in any order
      | a.name | a.birthyear |
      | null   | null        |