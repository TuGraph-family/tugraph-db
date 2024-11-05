Feature: test agg
  Scenario: case01
    Given yago graph
    When executing query
      '''
      match (n) return count(n);
      '''
    Then the result should be, in any order
      | count(n) |
      | 21       |
    When executing query
      '''
      match (n) return count(n), 1;
      '''
    Then the result should be, in any order
      | count(n) | 1 |
      | 21       | 1 |
    When executing query
      '''
      match (n:Person) return avg(n.birthyear) as avg;
      '''
    Then the result should be, in any order
      | avg               |
      | 1939.923076923077 |
    When executing query
      '''
      match (n:Person) return max(n.birthyear) as max;
      '''
    Then the result should be, in any order
      | max    |
      | 1986.0 |
    When executing query
      '''
      match (n:Person) return min(n.birthyear) as min;
      '''
    Then the result should be, in any order
      | min    |
      | 1873.0 |
    When executing query
      '''
      match (n) delete n;
      '''
    Then the result should be, in any order
      | <SUMMARY>                              |
      | 'deleted 21 vertices, deleted 28 edges.' |
    When executing query
      '''
      match (n) return count(n);
      '''
    Then the result should be, in any order
      | count(n) |
      | 0        |
    When executing query
      '''
      match (n) return count(n), 1;
      '''
    Then the result should be, in any order
      | count(n) | 1 |
    When executing query
      '''
      match (n:Person) return avg(n.birthyear) as avg;
      '''
    Then the result should be, in any order
      | avg  |
      | null |
    When executing query
      '''
      match (n:Person) return max(n.birthyear) as max;
      '''
    Then the result should be, in any order
      | max  |
      | null |
    When executing query
      '''
      match (n:Person) return min(n.birthyear) as min;
      '''
    Then the result should be, in any order
      | min  |
      | null |