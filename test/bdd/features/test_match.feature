Feature: test match
  Scenario: case01
    Given an empty graph
    When executing query
      """
      MATCH (n)
      RETURN n
      """
    Then the result should be, in any order
      | n |

  Scenario: case02
    Given an empty graph
    And having executed:
      """
      CREATE (:A), (:B {name: 'b'}), ({name: 'c'})
      """
    When executing query
      """
      MATCH (n)
      RETURN n
      """
    Then the result should be, in any order
      | n                |
      | (:A)             |
      | (:B {name: 'b'}) |
      | ({name: 'c'})    |

  Scenario: case03
    Given an empty graph
    And having executed:
      """
      CREATE ({name: 'bar'}), ({name: 'monkey'}), ({firstname: 'bar'})
      """
    When executing query
      """
      MATCH (n {name: 'bar'})
      RETURN n
      """
    Then the result should be, in any order
      | n                |
      | ({name: 'bar'}) |

  Scenario: case04
    Given an empty graph
    And having executed:
      """
      CREATE ({num: 1}),
        ({num: 2}),
        ({num: 3})
      """
    When executing query
      """
      MATCH (n), (m)
      RETURN n.num AS n, m.num AS m
      """
    Then the result should be, in any order
      | n | m |
      | 1 | 1 |
      | 1 | 2 |
      | 1 | 3 |
      | 2 | 1 |
      | 2 | 2 |
      | 2 | 3 |
      | 3 | 3 |
      | 3 | 1 |
      | 3 | 2 |

  Scenario: case05
    Given an empty graph
    When executing query
      """
      MATCH (n $param)
      RETURN n
      """
    Then an Error should be raised

  Scenario: case06
    Given an empty graph
    When executing query
      """
      MATCH ()-[r]->()
      RETURN r
      """
    Then the result should be, in any order
      | r |

  Scenario: case07
    Given an empty graph
    And having executed
      """
      CREATE (:A)-[:T1]->(:B),
             (:B)-[:T2]->(:A),
             (:B)-[:T3]->(:B),
             (:A)-[:T4]->(:A)
      """
    When executing query
      """
      MATCH (:A)-[r]->(:B)
      RETURN r
      """
    Then the result should be, in any order
      | r     |
      | [:T1] |

  Scenario: case08
    Given an empty graph
    And having executed
      """
      CREATE (a)
      CREATE (a)-[:T]->(a)
      """
    When executing query
      """
      MATCH ()-[r]-()
      RETURN type(r) AS r
      """
    Then the result should be, in any order
      | r   |
      | 'T' |
      | 'T' |

  Scenario: case09
    Given an empty graph
    And having executed
      """
      CREATE (a)
      CREATE (a)-[:T]->(a)
      """
    When executing query
      """
      MATCH ()-[r]->()
      RETURN type(r) AS r
      """
    Then the result should be, in any order
      | r   |
      | 'T' |

  Scenario: case10
    Given an empty graph
    And having executed
      """
      CREATE (:A)<-[:KNOWS {name: 'monkey'}]-()-[:KNOWS {name: 'woot'}]->(:B)
      """
    When executing query
      """
      MATCH (node)-[r:KNOWS {name: 'monkey'}]->(a)
      RETURN a
      """
    Then the result should be, in any order
      | a    |
      | (:A) |

  Scenario: case11
    Given an empty graph
    And having executed
      """
      CREATE (a {name: 'A'}),
        (b {name: 'B'}),
        (c {name: 'C'}),
        (a)-[:KNOWS]->(b),
        (a)-[:HATES]->(c),
        (a)-[:WONDERS]->(c)
      """
    When executing query
      """
      MATCH (n)-[r:KNOWS|HATES]->(x)
      RETURN r
      """
    Then the result should be, in any order
      | r        |
      | [:KNOWS] |
      | [:HATES] |

  Scenario: case12
    Given an empty graph
    When executing query
      """
      MATCH ()-[r:FOO $param]->()
      RETURN r
      """
    Then an Error should be raised