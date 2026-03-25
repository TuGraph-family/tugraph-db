Feature: test index
  Scenario: case01 non-unique composite property index exact query
    Given an empty graph
    And having executed
      """
      CREATE (:person {id:1, country:'cn', name:'alice'});
      CREATE (:person {id:1, country:'cn', name:'alice_2'});
      CREATE (:person {id:1, country:'us', name:'bob'});
      CREATE (:person {id:2, country:'cn', name:'cindy'});
      CALL db.index.createNodeIndex('person_id_country', 'person', ['id', 'country'], {unique:false});
      """
    When executing query
      """
      CALL db.index.queryNodes('person_id_country', [1, 'cn']) YIELD node RETURN node.name
      """
    Then the result should be, in any order
      | node.name |
      | 'alice'   |
      | 'alice_2' |

  Scenario: case02 non-unique string and composite property index range query
    Given an empty graph
    And having executed
      """
      CREATE (:event {user_id:'user_001', timestamp:100, name:'event_a'});
      CREATE (:event {user_id:'user_002', timestamp:100, name:'event_b'});
      CREATE (:event {user_id:'user_002', timestamp:200, name:'event_c'});
      CREATE (:event {user_id:'user_002', timestamp:300, name:'event_d'});
      CREATE (:event {user_id:'user_003', timestamp:100, name:'event_e'});
      CREATE (:event {user_id:'user_003', timestamp:200, name:'event_f'});
      CREATE (:event {user_id:'user_004', timestamp:100, name:'event_g'});
      CALL db.index.createNodeIndex('event_user_id', 'event', ['user_id'], {unique:false});
      CALL db.index.createNodeIndex('event_user_id_timestamp', 'event', ['user_id', 'timestamp'], {unique:false});
      """
    When executing query
      """
      CALL db.index.rangeQueryNodes('event_user_id', 'user_002', 'user_004', {left_closed:true, right_closed:false}) YIELD node RETURN node.user_id, node.timestamp, node.name
      """
    Then the result should be, in any order
      | node.user_id | node.timestamp | node.name |
      | 'user_002'   | 100            | 'event_b' |
      | 'user_002'   | 200            | 'event_c' |
      | 'user_002'   | 300            | 'event_d' |
      | 'user_003'   | 100            | 'event_e' |
      | 'user_003'   | 200            | 'event_f' |
    When executing query
      """
      CALL db.index.rangeQueryNodes('event_user_id', 'user_002', 'user_002', {left_closed:true, right_closed:true}) YIELD node RETURN node.user_id, node.timestamp, node.name
      """
    Then the result should be, in any order
      | node.user_id | node.timestamp | node.name |
      | 'user_002'   | 100            | 'event_b' |
      | 'user_002'   | 200            | 'event_c' |
      | 'user_002'   | 300            | 'event_d' |
    When executing query
      """
      CALL db.index.rangeQueryNodes('event_user_id_timestamp', ['user_002', 200], ['user_003', 150], {left_closed:true, right_closed:false}) YIELD node RETURN node.user_id, node.timestamp, node.name
      """
    Then the result should be, in any order
      | node.user_id | node.timestamp | node.name |
      | 'user_002'   | 200            | 'event_c' |
      | 'user_002'   | 300            | 'event_d' |
      | 'user_003'   | 100            | 'event_e' |
    When executing query
      """
      CALL db.index.rangeQueryNodes('event_user_id_timestamp', ['user_002', 200], ['user_003', 150], {left_closed:true, right_closed:false}) YIELD node WHERE node.name = 'event_c' RETURN node.user_id, node.timestamp, node.name
      """
    Then the result should be, in any order
      | node.user_id | node.timestamp | node.name |
      | 'user_002'   | 200            | 'event_c' |
