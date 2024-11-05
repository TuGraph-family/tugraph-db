Feature: test create edge
  Scenario: case01
    Given an empty graph
    When executing query
      """
      CREATE ()-[:R]->()
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 1 edges.'  |

  Scenario: case02
    Given an empty graph
    When executing query
      """
      CREATE (a), (b), (a)-[:R]->(b)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 1 edges.'  |

  Scenario: case03
    Given an empty graph
    When executing query
      """
      CREATE (a)
      CREATE (b)
      CREATE (a)-[:R]->(b)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 1 edges.'  |

  Scenario: case04
    Given an empty graph
    And having executed
      """
      CREATE (:A)<-[:R]-(:B)
      """
    When executing query
      """
      MATCH (a:A)<-[:R]-(b:B) RETURN a, b;
      """
    Then the result should be, in any order
      | a    | b    |
      | (:A) | (:B) |

  Scenario: case05
    Given an empty graph
    And having executed
      """
      CREATE (:X)
      CREATE (:Y)
      """
    When executing query
      """
      MATCH (x:X), (y:Y)
      CREATE (x)-[:R]->(y)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 0 vertices, created 1 edges.'  |

  Scenario: case06
    Given an empty graph
    And having executed
      """
      CREATE (:X) CREATE (:Y);
      MATCH (x:X), (y:Y) CREATE (x)<-[:R]-(y)
      """
    When executing query
      """
      MATCH (x:X)<-[:R]-(y:Y)
      RETURN x, y
      """
    Then the result should be, in any order
      | x    |  y   |
      | (:X) | (:Y) |

  Scenario: case07
    Given an empty graph
    When executing query
      """
      CREATE (root)-[:LINK]->(root)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 1 edges.'  |

  Scenario: case08
    Given an empty graph
    When executing query
      """
      CREATE (root),(root)-[:LINK]->(root)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 1 edges.'  |

  Scenario: case09
    Given an empty graph
    When executing query
      """
      CREATE (root)
      CREATE (root)-[:LINK]->(root)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 1 vertices, created 1 edges.'  |

  Scenario: case10
    Given an empty graph
    And having executed
      """
      CREATE (:Root)
      """
    When executing query
      """
      MATCH (root:Root)
      CREATE (root)-[:LINK]->(root)
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 0 vertices, created 1 edges.'  |


  Scenario: case11
    Given an empty graph
    And having executed
      """
      CREATE (:Begin);
      MATCH (x:Begin) CREATE (x)-[:TYPE]->(:End);
      """
    When executing query
      """
      MATCH (x:Begin)-[:TYPE]->(y:End) RETURN x, y
      """
    Then the result should be, in any order
      | x        | y      |
      | (:Begin) | (:End) |

  Scenario: case12
    Given an empty graph
    And having executed
      """
      CREATE (:End);
      MATCH (x:End) CREATE (:Begin)-[:TYPE]->(x);
      """
    When executing query
      """
      MATCH (x:Begin)-[:TYPE]->(y:End) RETURN x, y
      """
    Then the result should be, in any order
      | x        | y      |
      | (:Begin) | (:End) |

  Scenario: case13
    Given an empty graph
    When executing query
      """
      CREATE ()-[:R {num: 42}]->()
      """
    Then the result should be, in any order
      | <SUMMARY> |
      | 'created 2 vertices, created 1 edges.'  |

  Scenario: case14
    Given an empty graph
    When executing query
      """
      CREATE ()-[r:R {num: 42}]->()
      RETURN r.num AS num
      """
    Then the result should be, in any order
      | num |
      | 42  |

  Scenario: case15
    Given an empty graph
    When executing query
      """
      CREATE ()-[r:R {id: 12, name: 'foo'}]->()
      RETURN r.id AS id, r.name AS name
      """
    Then the result should be, in any order
      | id | name  |
      | 12 | 'foo' |

  Scenario: case16
    Given an empty graph
    When executing query
      """
      CREATE ()-[r:X {id: 12, name: null}]->()
      RETURN r.id, r.name AS name
      """
    Then the result should be, in any order
      | r.id | name |
      | 12   | null |

  Scenario: case17
    Given an empty graph
    When executing query
      """
      CREATE ()-->()
      """
    Then an Error should be raised

  Scenario: case18
    Given an empty graph
    When executing query
      """
      CREATE (a)-[:FOO]-(b)
      """
    Then an Error should be raised

  Scenario: case19
    Given an empty graph
    When executing query
      """
      CREATE (a)<-[:FOO]->(b)
      """
    Then an Error should be raised

  Scenario: case20
    Given an empty graph
    When executing query
      """
      CREATE ()-[:A|:B]->()
      """
    Then an Error should be raised