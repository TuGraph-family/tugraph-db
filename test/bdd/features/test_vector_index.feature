Feature: test vector index
  Scenario: case01
    Given an empty graph
    And having executed
      """
      CREATE (n1:person {id:1, age:10, embedding: [1.0,1.0,1.0,1.0]})
      CREATE (n2:person {id:2, age:20, embedding: [2.0,2.0,2.0,2.0]})
      CREATE (n3:person {id:3, age:30, embedding: [3.0,3.0,3.0,3.0]})
      CREATE (n1)-[r:like]->(n2),
             (n2)-[r:like]->(n3),
             (n3)-[r:like]->(n1);
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      """
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node RETURN node
      """
    Then the result should be, in any order
      | node             |
      | (:person {age:20, embedding:[2.0,2.0,2.0,2.0], id:2})  |
      | (:person {age:30, embedding:[3.0,3.0,3.0,3.0], id:3})  |
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node where node.age > 20 RETURN node
      """
    Then the result should be, in any order
      | node             |
      | (:person {age:30, embedding:[3.0,3.0,3.0,3.0], id:3})  |
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2})
      YIELD node where node.age > 20 with node as p
      match(p)-[r]->(m) return m
      """
    Then the result should be, in any order
      | m             |
      | (:person {age:10, embedding:[1.0,1.0,1.0,1.0], id:1})  |
  Scenario: case02
    Given an empty graph
    And having executed
      """
      CREATE (n1:person {id:1, age:10, embedding: [1.0,1.0,1.0,1.0]})
      CREATE (n2:person {id:2, age:20, embedding: [2.0,2.0,2.0,2.0]})
      CREATE (n3:person {id:3, age:30, embedding: [3.0,3.0,3.0,3.0]})
      CREATE (n1)-[r:like]->(n2),
             (n2)-[r:like]->(n3),
             (n3)-[r:like]->(n1);
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      """
    When executing query
      """
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      """
    Then an Error should be raised
    When executing query
      """
      CALL db.index.vector.createNodeIndex('vector_index1','person', 'embedding', {dimension:4});
      """
    Then an Error should be raised
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index_non", [1.0,2.0,3.0,4.0], {top_k:2})
      YIELD node where node.age > 20 with node as p
      match(p)-[r]->(m) return m
      """
    Then an Error should be raised

  Scenario: case03
    Given an empty graph
    And having executed
      """
      CREATE (n1:person {id:1, age:10, embedding: [1.0,1.0,1.0,1.0]})
      CREATE (n2:person {id:2, age:20, embedding: [2.0,2.0,2.0,2.0]})
      CREATE (n3:person {id:3, age:30, embedding: [3.0,3.0,3.0,3.0]})
      CREATE (n1)-[r:like]->(n2),
             (n2)-[r:like]->(n3),
             (n3)-[r:like]->(n1);
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      """
    When executing query
      """
      CALL db.index.vector.deleteIndex('vector_index_non');
      """
    Then an Error should be raised
    When executing query
      """
      CALL db.index.vector.deleteIndex('vector_index');
      """
    Then the result should be empty
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2})
      YIELD node where node.age > 20 with node as p
      match(p)-[r]->(m) return m
      """
    Then an Error should be raised

  Scenario: case04
    Given an empty graph
    And having executed
      """
      CREATE (n1:person {id:1, age:10, embedding: toFloat32List([1.0,1.0,1.0,1.0])})
      CREATE (n2:person {id:2, age:20, embedding: toFloat32List([2.0,2.0,2.0,2.0])})
      CREATE (n3:person {id:3, age:30, embedding: toFloat32List([3.0,3.0,3.0,3.0])})
      CREATE (n1)-[r:like]->(n2),
             (n2)-[r:like]->(n3),
             (n3)-[r:like]->(n1);
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      """
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node RETURN node
      """
    Then the result should be, in any order
      | node             |
      | (:person {age:20, embedding:[2.0,2.0,2.0,2.0], id:2})  |
      | (:person {age:30, embedding:[3.0,3.0,3.0,3.0], id:3})  |

  Scenario: case05
    Given an empty graph
    And having executed
      """
      CREATE (n1:person {id:1, age:10})
      CREATE (n2:person {id:2, age:20})
      CREATE (n3:person {id:3, age:30})
      CREATE (n1)-[r:like]->(n2),
             (n2)-[r:like]->(n3),
             (n3)-[r:like]->(n1);
      CALL db.index.vector.createNodeIndex('vector_index','person', 'embedding', {dimension:4});
      MATCH(n:person {id:1}) set n.embedding = toFloat32List([1.0,1.0,1.0,1.0]);
      MATCH(n:person {id:2}) set n.embedding = toFloat32List([2.0,2.0,2.0,2.0]);
      MATCH(n:person {id:3}) set n.embedding = toFloat32List([3.0,3.0,3.0,3.0]);
      CALL db.index.vector.applyWal();
      """
    When executing query
      """
      CALL db.index.vector.knnSearchNodes("vector_index", [1.0,2.0,3.0,4.0], {top_k:2}) YIELD node RETURN node
      """
    Then the result should be, in any order
      | node             |
      | (:person {age:20, embedding:[2.0,2.0,2.0,2.0], id:2})  |
      | (:person {age:30, embedding:[3.0,3.0,3.0,3.0], id:3})  |