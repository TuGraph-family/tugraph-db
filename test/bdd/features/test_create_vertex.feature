Feature: test create vertex
  Scenario: case01
    Given an empty graph
    When executing query
      """
      CREATE ()
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |
  Scenario: case02
    Given an empty graph
    When executing query
      """
      CREATE (), ()
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 0 edges.'  |

  Scenario: case03
    Given an empty graph
    When executing query
      """
      CREATE (:Label)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |

  Scenario: case04
    Given an empty graph
    When executing query
      """
      CREATE (:Label), (:Label)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 0 edges.'  |

  Scenario: case05
    Given an empty graph
    When executing query
      """
      CREATE (:A:B:C:D)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |

  Scenario: case06
    Given an empty graph
    When executing query
      """
      CREATE (:B:A:D), (:B:C), (:D:E:B)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 3 vertices, created 0 edges.'  |

  Scenario: case07
    Given an empty graph
    When executing query
      """
      CREATE ({created: true})
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |

  Scenario: case08
    Given an empty graph
    When executing query
      """
      CREATE (n {name: 'foo'})
      RETURN n.name AS p
      """
    Then the result should be, in any order
      | p     |
      | 'foo' |

  Scenario: case09
    Given an empty graph
    When executing query
      """
      CREATE (n {id: 12, name: 'foo'})
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 0 edges.'  |

  Scenario: case10
    Given an empty graph
    When executing query
      """
      CREATE (n {id: 12, name: 'foo'})
      RETURN n.id AS id, n.name AS p
      """
    Then the result should be, in any order
      | id | p     |
      | 12 | 'foo' |

  Scenario: case11
    Given an empty graph
    When executing query
      """
      CREATE (n {id: 12, name: null})
      RETURN n.id AS id, n.name AS p
      """
    Then the result should be, in any order
      | id | p    |
      | 12 | null |

  Scenario: case12
    Given an empty graph
    When executing query
      """
      CREATE (p:TheLabel {id: 4611686018427387905})
      RETURN p.id
      """
    Then the result should be, in any order
      | p.id                |
      | 4611686018427387905 |

  Scenario: case13
    Given an empty graph
    When executing query
      """
      CREATE (p:node1 {
              bool:true,
              int:100,
              double:1.2345,
              str:'test',
              bool_array:[true,false],
              int_array:[1,2,3,4],
              double_array:[1.1, 2.2],
              str_array:['str1','str2']})
      RETURN p
      """
    Then the result should be, in any order
      | p                |
      | (:node1 {bool: true, int:100, double:1.2345, str:'test', bool_array:[True,False], int_array:[1,2,3,4], double_array:[1.1, 2.2], str_array:['str1','str2']}) |

  Scenario: case14
    Given an empty graph
    When executing query
      """
      CREATE (p:node1 {array:[true,false,1,'test']})
      RETURN p
      """
    Then an Error should be raised

  Scenario: case15
    Given an empty graph
    When executing query
      """
      CREATE (p:node1 {map:{key:1}})
      RETURN p
      """
    Then an Error should be raised