Feature: test openspg
  Scenario: case01
    Given yago graph
    When executing query
      """
      CALL db.labels()
      """
    Then the result should be, in any order
      | label |
      |  'Person' |
      |  'City' |
      |  'Film' |

  Scenario: case02
    Given yago graph
    When executing query
      """
      CALL db.relationshipTypes()
      """
    Then the result should be, in any order
      | relationshipType |
      |  'HAS_CHILD' |
      |  'MARRIED' |
      |  'BORN_IN' |
      |  'WROTE_MUSIC_FOR' |
      |  'DIRECTED' |
      |  'ACTED_IN' |

  Scenario: case03
    Given yago graph
    When executing query
      """
      CALL dbms.graph.createGraph('graph1')
      """
    Then the result should be empty

  Scenario: case04
    Given yago graph
    When executing query
      """
      match(n) return count(n)
      """
    Then the result should be, in any order
    | count(n) |
    |    21    |
    When executing query
      """
      match(n)-[r]->(m) return count(r)
      """
    Then the result should be, in any order
      | count(r)  |
      |    28     |

    When executing query
      """
      CALL db.dropDB()
      """
    Then the result should be empty
    When executing query
      """
      match(n) return count(n)
      """
    Then the result should be, in any order
      | count(n) |
      |    0     |
    When executing query
      """
      match(n)-[r]->(m) return count(r)
      """
    Then the result should be, in any order
      | count(r)  |
      |    0     |

  Scenario: case05
    Given yago graph
    When executing query
      """
      MERGE (n:label {id: 100}) SET n += {name:'bob', age:20} RETURN n
      """
    Then the result should be, in any order
    | n  |
    |  (:label{age:20,id:100,name:'bob'})  |
    When executing query
      """
      MERGE (n:label {id: 100}) SET n += {country:'china', city:'sahnghai'} RETURN n
      """
    Then the result should be, in any order
      | n  |
      |  (:label{age:20,city:'sahnghai',country:'china',id:100,name:'bob'})  |

  Scenario: case06
    Given yago graph
    When executing query
      """
      UNWIND [{id_key:1, name:'name1'},{id_key:2, name:'name2'}] AS properties
      MERGE (n:label {id_key: properties.id_key}) SET n += properties RETURN n
      """
    Then the result should be, in any order
      | n  |
      | (:label{id_key:1,name:'name1'}) |
      | (:label{id_key:2,name:'name2'}) |
    When executing query
      """
      UNWIND [{id_key:1, age:11},{id_key:2, age:22}] AS properties
      MERGE (n:label {id_key: properties.id_key}) SET n += properties RETURN n
      """
    Then the result should be, in any order
      | n  |
      | (:label{age:11,id_key:1,name:'name1'}) |
      | (:label{age:22,id_key:2,name:'name2'}) |

  Scenario: case07
    Given yago graph
    When executing query
      """
      MATCH (n:Person {name: 'Rachel Kempson'}) RETURN n
      """
    Then the result should be, in any order
      | n  |
      | (:Person{birthyear:1910,name:'RachelKempson'}) |

  Scenario: case08
    Given yago graph
    When executing query
      """
      MATCH (n:Person {name: 'Rachel Kempson'}) DETACH DELETE n;
      """
    Then the result should be, in any order
      | <SUMMARY>  |
      | 'deleted 1 vertices,deleted 4 edges.' |

  Scenario: case09
    Given yago graph
    When executing query
      """
      UNWIND ['Rachel Kempson'] AS id_value MATCH (n:Person {name: id_value}) DETACH DELETE n
      """
    Then the result should be, in any order
      | <SUMMARY>  |
      | 'deleted 1 vertices,deleted 4 edges.' |

  Scenario: case10
    Given yago graph
    When executing query
      """
      MATCH (n:Person) RETURN count(n)
      """
    Then the result should be, in any order
      | count(n)  |
      | 13 |

  Scenario: case11
    Given yago graph
    When executing query
      """
      MERGE (a:start_node_label {start_node_id_key: 1})
      MERGE (b:end_node_label {end_node_id_key: 2})
      MERGE (a)-[r:rel_type]->(b) SET r += {score:2, id:1} RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{id:1,score:2}] |
    When executing query
      """
      MERGE (a:start_node_label {start_node_id_key: 1})
      MERGE (b:end_node_label {end_node_id_key: 2})
      MERGE (a)-[r:rel_type]->(b) SET r += {score:3} RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{id:1,score:3}] |

  Scenario: case12
    Given yago graph
    When executing query
      """
      MERGE (a:start_node_label {start_node_id_key: 1})
      MERGE (b:end_node_label {end_node_id_key: 2})
      """
    Then the result should be, in any order
      | <SUMMARY>  |
      | 'merged 2 vertices,merged 0 edges.' |
    When executing query
      """
      MATCH (a:start_node_label {start_node_id_key: 1}),
      (b:end_node_label {end_node_id_key: 2})
      MERGE (a)-[r:rel_type]->(b) SET r += {score:1} RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{score:1}] |

  Scenario: case13
    Given yago graph
    When executing query
      """
      UNWIND [{start_node_id:1, end_node_id:2, properties:{score:3}}] AS relationship
      MERGE (a:start_node_label {start_node_id_key: relationship.start_node_id})
      MERGE (a:end_node_label {end_node_id_key: relationship.end_node_id})
      MERGE (a)-[r:rel_type]->(b) SET r += relationship.properties RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{score:3}] |

  Scenario: case14
    Given yago graph
    When executing query
      """
      MERGE (a:start_node_label {start_node_id_key: 1})
      MERGE (b:end_node_label {end_node_id_key: 2})
      """
    When executing query
      """
      UNWIND [{start_node_id:1, end_node_id:2, properties:{score:3}}] AS relationship
      MATCH (a:start_node_label {start_node_id_key: relationship.start_node_id}), (b:end_node_label {end_node_id_key: relationship.end_node_id})
      MERGE (a)-[r:rel_type]->(b) SET r += relationship.properties RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{score:3}] |

  Scenario: case15
    Given yago graph
    When executing query
      """
      MERGE (a:start_node_label {start_node_id_key: 1})
      MERGE (b:end_node_label {end_node_id_key: 2})
      """
    When executing query
      """
      UNWIND [{start_node_id:1, end_node_id:2, properties:{score:3}}] AS relationship
      MATCH (a:start_node_label {start_node_id_key: relationship.start_node_id})
      MATCH (b:end_node_label {end_node_id_key: relationship.end_node_id})
      MERGE (a)-[r:rel_type]->(b) SET r += relationship.properties RETURN r
      """
    Then the result should be, in any order
      | r  |
      | [:rel_type{score:3}] |

  Scenario: case16
    Given yago graph
    When executing query
      """
      MATCH (a:Person {name:'Rachel Kempson'})-[r:HAS_CHILD]->(b:Person {name:'Vanessa Redgrave'})
      DELETE r
      """
    Then the result should be, in any order
      | <SUMMARY>  |
      | 'deleted 0 vertices,deleted 1 edges.' |

  Scenario: case17
    Given yago graph
    When executing query
      """
      UNWIND [{start_name:'Rachel Kempson', end_name:'Vanessa Redgrave'},{start_name:'Rachel Kempson', end_name:'Corin Redgrave'}] AS arg
      MATCH (a:Person {name: arg.start_name})-[r:HAS_CHILD]->(b:Person {name: arg.end_name}) DELETE r
      """
    Then the result should be, in any order
      | <SUMMARY>  |
      | 'deleted 0 vertices,deleted 2 edges.' |

  Scenario: case18
    Given yago graph
    When executing query
      """
      CALL db.createUniquePropertyConstraint('person_name_unique', 'Person', 'name')
      """
    Then the result should be empty

  Scenario: case19
    Given yago graph
    When executing query
      """
      MATCH(n) where (n:Person) return count(n)
      """
    Then the result should be, in any order
    | count(n) |
    |    13     |

  Scenario: case20
    Given an empty graph
    And having executed
    """
      CREATE
        (home:Page {name:'Home'}),
        (about:Page {name:'About'}),
        (product:Page {name:'Product'}),
        (links:Page {name:'Links'}),
        (a:Page {name:'Site A'}),
        (b:Page {name:'Site B'}),
        (c:Page {name:'Site C'}),
        (d:Page {name:'Site D'}),

        (home)-[:LINKS {weight: 0.2}]->(about),
        (home)-[:LINKS {weight: 0.2}]->(links),
        (home)-[:LINKS {weight: 0.6}]->(product),
        (about)-[:LINKS {weight: 1.0}]->(home),
        (product)-[:LINKS {weight: 1.0}]->(home),
        (a)-[:LINKS {weight: 1.0}]->(home),
        (b)-[:LINKS {weight: 1.0}]->(home),
        (c)-[:LINKS {weight: 1.0}]->(home),
        (d)-[:LINKS {weight: 1.0}]->(home),
        (links)-[:LINKS {weight: 0.8}]->(home),
        (links)-[:LINKS {weight: 0.05}]->(a),
        (links)-[:LINKS {weight: 0.05}]->(b),
        (links)-[:LINKS {weight: 0.05}]->(c),
        (links)-[:LINKS {weight: 0.05}]->(d);
    """
    When executing query
      """
        CALL algo.pagerank({num_iterations:20}) yield node,score return node.name as name, score ORDER BY score DESC, name ASC
      """
    Then the result should be, in any order
      | name    | score               |
      | 'Home'    | 0.5136798070889166  |
      | 'About'   | 0.16902265561073537 |
      | 'Links'   | 0.16902265561073537 |
      | 'Product' | 0.16902265561073537 |
      | 'Site A'  | 0.04843040785329749 |
      | 'Site B'  | 0.04843040785329749 |
      | 'Site C'  | 0.04843040785329749 |
      | 'Site D'  | 0.04843040785329749 |
