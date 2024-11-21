Feature: test fulltext index
  Scenario: case01
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils Johansson'      |
      | 'Nils-Erik Karlsson'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", '"Nils-Erik"', 10) YIELD node
      RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils-Erik Karlsson'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", 'nils AND kernel', 10) YIELD node
      RETURN node.name, node.team
      """
    Then the result should be, in any order
      | node.name             | node.team |
      | 'Nils-Erik Karlsson'  | 'Kernel'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node
      RETURN node.name, node.team
      """
    Then the result should be, in any order
      | node.name             | node.team     |
      | 'Nils Johansson'      | 'Operations'  |
      | 'Maya Tanaka'         | 'Operations'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka'
      RETURN node.name, node.team
      """
    Then the result should be, in any order
      | node.name             | node.team     |
      | 'Maya Tanaka'         | 'Operations'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", 'team:"Operations"', 10) YIELD node where node.name = 'Maya Tanaka'
      with node
      MATCH(node)-[r]->(m)
      return m
      """
    Then the result should be, in any order
      | m    |
      | (:Employee{name:'NilsJohansson',position:'Engineer',team:'Operations'})|
    When executing query
      """
      MATCH(n:Manager {name:'Lisa Danielsson'})
      with n
      CALL db.index.fulltext.queryNodes("namesAndTeams", substring(n.name,1,4), 10) YIELD node where node.name = 'Lisa Danielsson'
      with node
      MATCH(node)-[r]->(m)
      return m.name
      """
    Then the result should be, in any order
      | m.name    |
      | 'Nils-ErikKarlsson' |

  Scenario: case02
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      match (n {name:'Nils Johansson'}) delete n;
      CALL db.index.fulltext.applyWal();
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils-Erik Karlsson'  |
    When executing query
      """
      create (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"})
      """
    When executing query
      """
      CALL db.index.fulltext.applyWal();
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils-Erik Karlsson'  |
      | 'Nils Johansson'      |

  Scenario: case03
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      match (n {name:'Nils Johansson'}) remove n.name;
      CALL db.index.fulltext.applyWal();
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils-Erik Karlsson'  |
    When executing query
      """
      match (n {position:'Engineer', team:'Operations'}) set n.name = 'Nils Johansson';
      """
    When executing query
      """
      CALL db.index.fulltext.applyWal();
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Nils Johansson'      |
      | 'Nils-Erik Karlsson'  |

  Scenario: case04
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      """
    When executing query
      """
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      """
    Then an Error should be raised

  Scenario: case05
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager"}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations"}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('namesAndTeams',['Employee','Manager'], ['name','team']);
      """
    When executing query
      """
      CALL db.index.fulltext.deleteIndex('namesAndTeams_non');
      """
    Then an Error should be raised
    When executing query
      """
      CALL db.index.fulltext.deleteIndex('namesAndTeams');
      """
    Then the result should be empty
    When executing query
      """
      CALL db.index.fulltext.queryNodes("namesAndTeams", "nils", 10) YIELD node, score RETURN node.name
      """
    Then an Error should be raised

  Scenario: case05
    Given an empty graph
    And having executed
      """
      CREATE (nilsE:Employee {name: "Nils-Erik Karlsson", position: "Engineer", team: "Kernel", peerReviews: ['Nils-Erik is difficult to work with.', 'Nils-Erik is often late for work.']}),
      (lisa:Manager {name: "Lisa Danielsson", position: "Engineering manager", interest:'football'}),
      (nils:Employee {name: "Nils Johansson", position: "Engineer", team: "Operations", interest:'football'}),
      (maya:Employee {name: "Maya Tanaka", position: "Senior Engineer", team:"Operations"}),
      (lisa)-[:REVIEWED {message: "Nils-Erik is reportedly difficult to work with."}]->(nilsE),
      (maya)-[:EMAILED {message: "I have booked a team meeting tomorrow."}]->(nils);
      CALL db.index.fulltext.createNodeIndex('ft_index',['Employee','Manager'], ['name','team', 'interest']);
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("ft_index", "football", 10) YIELD node
      WHERE (node:Manager)
      RETURN node.name
      """
    Then the result should be, in any order
      | node.name             |
      | 'Lisa Danielsson'      |

  Scenario: case06
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('Chunk_id', 'Chunk', 'id');
      CALL db.index.fulltext.createNodeIndex('Chunk_tags',['Chunk'], ['tags']);
      CREATE(n1:Chunk {id:1, tags:'keyword1 keyword2 keyword3 keyword4'})
      CREATE(n2:Chunk {id:2, tags:'keyword5 keyword6 keyword7 keyword8'});
      CALL db.index.fulltext.applyWal();
      """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("Chunk_tags", "keyword3", 10) YIELD node return node.id
      """
    Then the result should be, in any order
      | node.id |
      | 1       |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("Chunk_tags", "keyword7", 10) YIELD node return node.id
      """
    Then the result should be, in any order
      | node.id |
      | 2       |

  Scenario: case07
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('Chunk_id', 'Chunk', 'id');
      CALL db.index.fulltext.createNodeIndex('Chunk_tags',['Chunk'], ['tags']);
      """
    When executing query
      """
      CREATE(n1:Chunk {id:1, tags:'keyword1 keyword2 keyword3 keyword4'})
      CREATE(n2:Chunk {id:1, tags:'keyword5 keyword6 keyword7 keyword8'});
      """
    Then an Error should be raised
    When executing query
    """
    CALL db.index.fulltext.applyWal();
    """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("Chunk_tags", "keyword3", 10) YIELD node return node.id
      """
    Then the result should be empty
    When executing query
      """
      CALL db.index.fulltext.queryNodes("Chunk_tags", "keyword7", 10) YIELD node return node.id
      """
    Then the result should be empty
