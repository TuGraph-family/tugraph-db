Feature: test opt
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH ()-[r]->() RETURN count(r);
      '''
    Then the result should be, in any order
      | count(r) |
      | 28       |
    When executing query
      '''
      MATCH (:Person)-[r]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 28       |
    When executing query
      '''
      MATCH (:Film)-[r]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0       |
    When executing query
      '''
      MATCH ()-[r]->(:Person) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11       |
    When executing query
      '''
      MATCH ()-[r]->(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11       |
    When executing query
      '''
      MATCH (:Person)-[r]->(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11       |
    When executing query
      '''
      MATCH (:Person)-[r]->(:NO_LABEL) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH ()-[r:MARRIED]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 4        |
    When executing query
      '''
      MATCH ()-[r:NO_LABEL]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 10        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN|NO_LABEL]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 10        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 10        |
    When executing query
      '''
      MATCH (:City)-[r:MARRIED|BORN_IN]->() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN]->(:Person) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 4        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]->(:City) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 6        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]->(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH (:Person)-[r:DIRECTED]->(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 1        |
    When executing query
      '''
      MATCH (:Person)-[r:DIRECTED|ACTED_IN]->(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 9        |
    When executing query
      '''
      MATCH ()-[r]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 56        |
    When executing query
      '''
      MATCH (:Person)-[r]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 39        |
    When executing query
      '''
      MATCH (:Film)-[r]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11        |
    When executing query
      '''
      MATCH ()-[r]-(:Person) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 39        |
    When executing query
      '''
      MATCH ()-[r]-(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11        |
    When executing query
      '''
      MATCH (:Person)-[r]-(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 11        |
    When executing query
      '''
      MATCH (:Person)-[r]-(:NO_LABEL) RETURN count(r);
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH ()-[r:MARRIED]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 8        |
    When executing query
      '''
      MATCH ()-[r:NO_LABEL]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 20        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN|NO_LABEL]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 20        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 14        |
    When executing query
      '''
      MATCH (:City)-[r:MARRIED|BORN_IN]-() RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 6        |
    When executing query
      '''
      MATCH ()-[r:MARRIED|BORN_IN]-(:Person) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 14        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]-(:City) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 6        |
    When executing query
      '''
      MATCH (:Person)-[r:MARRIED|BORN_IN]-(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      MATCH (:Person)-[r:DIRECTED]-(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 1        |
    When executing query
      '''
      MATCH (:Person)-[r:DIRECTED|ACTED_IN]-(:Film) RETURN count(r)
      '''
    Then the result should be, in any order
      | count(r) |
      | 9        |