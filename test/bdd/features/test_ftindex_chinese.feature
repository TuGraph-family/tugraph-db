Feature: test fulltext index chinese
  Scenario: case01
    Given an empty graph
    And having executed
    """
      CREATE(n1:Chunk {id:1, tags:'["慢性病", "急性病", "糖尿病"]'})
      CREATE(n2:Chunk {id:2, tags:'["‌传染病", "‌恶性肿瘤", "‌动脉瘤"]'})
      CREATE(n3:Chunk {id:3, tags:'["‌传染病", "‌恶性肿瘤", "‌高血压"]'});
      CALL db.index.fulltext.createNodeIndex('ChunkTags',['Chunk'], ['tags']);
    """
    When executing query
      """
      CALL db.index.fulltext.queryNodes("ChunkTags", "‌恶性肿瘤", 10) YIELD node, score RETURN node.tags
      """
    Then the result should be, in any order
      | node.tags             |
      | '["‌传染病", "‌恶性肿瘤", "‌动脉瘤"]'  |
      | '["‌传染病", "‌恶性肿瘤", "‌高血压"]'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("ChunkTags", "糖尿病", 10) YIELD node, score RETURN node.tags
      """
    Then the result should be, in any order
      | node.tags             |
      | '["慢性病", "急性病", "糖尿病"]'  |

    When executing query
      """
      CALL db.index.fulltext.queryNodes("ChunkTags", "慢性病 OR 高血压", 10) YIELD node, score RETURN node.tags
      """
    Then the result should be, in any order
      | node.tags             |
      | '["慢性病", "急性病", "糖尿病"]'  |
      | '["‌传染病", "‌恶性肿瘤", "‌高血压"]'  |
    When executing query
      """
      CALL db.index.fulltext.queryNodes("ChunkTags", '传染病 AND ‌动脉瘤', 10) YIELD node, score RETURN node.tags
      """
    Then the result should be, in any order
      | node.tags             |
      | '["‌传染病", "‌恶性肿瘤", "‌动脉瘤"]'  |