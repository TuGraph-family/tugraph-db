Feature: test procedure
  Scenario: case01
    Given an initialized database
    When executing query
      """
      CALL dbms.graph.listGraph()
      """
    Then the result should be, in any order
      | id | name      |
      | 1  | 'default' |

  Scenario: case02
    Given an initialized database
    And having executed
      """
      CALL dbms.graph.createGraph('graph1')
      """
    When executing query
      """
      CALL dbms.graph.listGraph()
      """
    Then the result should be, in any order
      | id | name      |
      | 1  | 'default' |
      | 2  | 'graph1'  |

  Scenario: case03
    Given an initialized database
    And having executed
      """
      CALL dbms.graph.createGraph('graph1')
      """
    When executing query
      """
      CALL dbms.graph.listGraph() yield name return name
      """
    Then the result should be, in any order
      | name      |
      | 'default' |
      | 'graph1'  |

  Scenario: case04
    Given an initialized database
    And having executed
      """
      CALL dbms.graph.createGraph('graph1')
      """
    When executing query
      """
      CALL dbms.graph.listGraph() yield name where name = 'graph1' return name
      """
    Then the result should be, in any order
      | name      |
      | 'graph1'  |

  Scenario: case05
    Given an initialized database
    And having executed
      """
      CALL dbms.graph.createGraph('graph1');
      CALL dbms.graph.createGraph('graph2');
      CALL dbms.graph.deleteGraph('graph1');
      """
    When executing query
      """
      CALL dbms.graph.listGraph()
      """
    Then the result should be, in any order
      | id | name      |
      | 1  | 'default' |
      | 3  | 'graph2'  |

  Scenario: case06
    Given an initialized database
    Given yago graph
    When executing query
      """
      CALL db.labels()
      """
    Then the result should be, in any order
      | label      |
      | 'Person'  |
      | 'City'  |
      | 'Film'  |
    When executing query
      """
      CALL db.relationshipTypes()
      """
    Then the result should be, in any order
      | relationshipType      |
      | 'HAS_CHILD'  |
      | 'MARRIED'  |
      | 'BORN_IN'  |
      | 'WROTE_MUSIC_FOR'  |
      | 'DIRECTED'  |
      | 'ACTED_IN'  |

  Scenario: case07
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('person_id', 'person', 'id');
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      CALL db.index.vector.createNodeIndex('person_embedding','person', 'embedding', {dimension:4});
      """
    When executing query
      """
      CALL db.showIndexes()
      """
    Then the result should be, in any order
      | name | type | entityType | labelsOrTypes | properties |otherInfo                           |
      | 'person_id' | 'Unique' | 'NODE' | ['person'] | ['id'] |null                                |
      | 'namesAndTeams' | 'FullText' | 'NODE' | ['Employee','Manager'] | ['name','team'] |null                                |
      | 'person_embedding' | 'Vector' | 'NODE' | ['person'] | ['embedding'] | {deletedIdsNum:0, elementsNum:0, shardingNum:10} |

  Scenario: case08
    Given yago graph
    And having executed
    """
      CALL db.createUniquePropertyConstraint('person_name', 'Person', 'name');
    """
    When executing query
      """
      CALL db.showIndexes()
      """
    Then the result should be, in any order
      | name | type | entityType | labelsOrTypes | properties |otherInfo                           |
      | 'person_name' | 'Unique' | 'NODE' | ['Person'] | ['name'] |null                                |
    When executing query
      """
      CALL db.deleteUniquePropertyConstraint('person_name');
      """
    Then the result should be empty