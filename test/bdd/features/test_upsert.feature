Feature: test upsert
  Scenario: case01
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
      """
    When executing query
      """
      UNWIND [
        {id: '1', name: 'Alice', age: 30},
        {id: '2', name: 'Bob', age: 25},
        {id: '3', name: 'Charlie', age: 35}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n.name = record.name, n.age = record.age
      ON MATCH SET n.name = record.name, n.age = record.age
      """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:30,id:'1',name:'Alice'}) |
      | (:Person{age:25,id:'2',name:'Bob'}) |
      | (:Person{age:35,id:'3',name:'Charlie'}) |
    When executing query
      """
      UNWIND [
        {id: '1', name: 'Alice', age: 20},
        {id: '2', name: 'Bob', age: 15},
        {id: '3', name: 'Charlie', age: 25}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n.name = record.name, n.age = record.age
      ON MATCH SET n.name = record.name, n.age = record.age
      """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:20,id:'1',name:'Alice'}) |
      | (:Person{age:15,id:'2',name:'Bob'}) |
      | (:Person{age:25,id:'3',name:'Charlie'}) |
    When executing query
      """
      UNWIND [
        {id: '1', name: 'Alice', age: 20},
        {id: '2', name: 'Bob', age: 15},
        {id: '3', name: 'Charlie', age: 25}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n.name = record.name, n.age = record.age
      ON MATCH SET n.name = record.name, n.age = record.age
      """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:20,id:'1',name:'Alice'}) |
      | (:Person{age:15,id:'2',name:'Bob'}) |
      | (:Person{age:25,id:'3',name:'Charlie'}) |
    When executing query
      """
      UNWIND [
        {id: '1', country:'Canada'},
        {id: '2', country:'Brazil'},
        {id: '3', country:'United States'}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n.country = record.country
      ON MATCH SET n.country = record.country
      """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:20,country:'Canada',id:'1',name:'Alice'}) |
      | (:Person{age:15,country:'Brazil',id:'2',name:'Bob'}) |
      | (:Person{age:25,country:'United States',id:'3',name:'Charlie'}) |

  Scenario: case02
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
      UNWIND [
        {id: 1, name: 'Alice', age: 30},
        {id: 2, name: 'Bob', age: 25},
        {id: 3, name: 'Charlie', age: 35}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n.name = record.name, n.age = record.age
      ON MATCH SET n.name = record.name, n.age = record.age;
      """
    When executing query
    """
    UNWIND [
      {from: 1, to: 2, since: 2020},
      {from: 2, to: 3, since: 2021}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel.since = friendship.since
    ON MATCH SET rel.since = friendship.since;
    """
    When executing query
    """
    MATCH(n)-[r]->(m) return n.id, r ,m.id;
    """
    Then the result should be, in any order
      | n.id | r | m.id |
      | 1 | [:FRIEND_WITH {since:2020}] | 2 |
      | 2 | [:FRIEND_WITH {since:2021}] | 3 |

    When executing query
    """
    UNWIND [
      {from: 1, to: 2, since: 2020},
      {from: 2, to: 3, since: 2021}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel.since = friendship.since
    ON MATCH SET rel.since = friendship.since;
    """
    When executing query
    """
    MATCH(n)-[r]->(m) return n.id, r ,m.id;
    """
    Then the result should be, in any order
      | n.id | r | m.id |
      | 1 | [:FRIEND_WITH {since: 2020}] | 2 |
      | 2 | [:FRIEND_WITH{since: 2021} ] | 3 |
    When executing query
    """
    UNWIND [
      {from: 1, to: 3, since: 2022},
      {from: 3, to: 2, since: 2023}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel.since = friendship.since
    ON MATCH SET rel.since = friendship.since;
    """
    When executing query
    """
    MATCH(n)-[r]->(m) return n.id, r ,m.id;
    """
    Then the result should be, in any order
      | n.id | r | m.id |
      | 1 | [:FRIEND_WITH {since: 2020}] | 2 |
      | 1 | [:FRIEND_WITH {since:2022}] | 3 |
      | 2 | [:FRIEND_WITH {since:2021}] | 3 |
      | 3 | [:FRIEND_WITH {since:2023}] | 2 |

  Scenario: case03
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
      UNWIND [
        {id: 1, properties:{name: 'Alice', age: 30}},
        {id: 2, properties:{name: 'Bob', age: 25}},
        {id: 3, properties:{name: 'Charlie', age: 35}}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n += record.properties
      ON MATCH SET n += record.properties;
      """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:30,id:1,name:'Alice'}) |
      | (:Person{age:25,id:2,name:'Bob'}) |
      | (:Person{age:35,id:3,name:'Charlie'}) |
    When executing query
    """
      UNWIND [
        {id: 1, properties:{age: 20}},
        {id: 2, properties:{age: 15}},
        {id: 3, properties:{age: 25}}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n += record.properties
      ON MATCH SET n += record.properties;
    """
    When executing query
    """
    MATCH(n) return n;
    """
    Then the result should be, in any order
      | n |
      | (:Person{age:20,id:1,name:'Alice'}) |
      | (:Person{age:15,id:2,name:'Bob'}) |
      | (:Person{age:25,id:3,name:'Charlie'}) |

  Scenario: case04
    Given an empty graph
    And having executed
      """
      CALL db.createUniquePropertyConstraint('person_id', 'Person', 'id');
      UNWIND [
        {id: 1, properties:{name: 'Alice', age: 30}},
        {id: 2, properties:{name: 'Bob', age: 25}},
        {id: 3, properties:{name: 'Charlie', age: 35}}
      ] AS record
      MERGE (n:Person {id: record.id})
      ON CREATE SET n += record.properties
      ON MATCH SET n += record.properties;
      """
    When executing query
    """
    UNWIND [
      {from: 1, to: 2, properties:{since: 2020}},
      {from: 2, to: 3, properties:{since: 2021}}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel += friendship.properties
    ON MATCH SET rel += friendship.properties;
    """
    When executing query
    """
    MATCH(n)-[r]->(m) return n.id, r ,m.id;
    """
    Then the result should be, in any order
      | n.id | r | m.id |
      | 1 | [:FRIEND_WITH {since: 2020}] | 2 |
      | 2 | [:FRIEND_WITH{since: 2021} ] | 3 |
    When executing query
    """
    UNWIND [
      {from: 1, to: 2, properties:{since: 2030}},
      {from: 1, to: 3, properties:{since: 2024}}
    ] AS friendship
    MATCH (p1:Person {id: friendship.from})
    MATCH (p2:Person {id: friendship.to})
    MERGE (p1)-[rel:FRIEND_WITH]->(p2)
    ON CREATE SET rel += friendship.properties
    ON MATCH SET rel += friendship.properties;
    """
    When executing query
    """
    MATCH(n)-[r]->(m) return n.id, r ,m.id;
    """
    Then the result should be, in any order
      | n.id | r | m.id |
      | 1 | [:FRIEND_WITH {since: 2030}] | 2 |
      | 2 | [:FRIEND_WITH{since: 2021} ] | 3 |
      | 1 | [:FRIEND_WITH{since: 2024} ] | 3 |