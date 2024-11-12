Feature: test multi label
  Scenario: case01
    Given an empty graph
    And having executed
    """
    CREATE (n:Person {id:1});
    CREATE (n:Animal {id:2});
    """
    When executing query
      """
      match(n {id:1}) set n:Student;
      """
    When executing query
      """
      match(n) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Person:Student {id:1}) |
      | (:Animal {id:2}) |
    When executing query
      """
      match(n {id:1}) set n:Student;
      """
    When executing query
      """
      match(n) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Person:Student {id:1}) |
      | (:Animal {id:2}) |
    When executing query
      """
      match(n {id:1}) set n:Student:Musician;
      """
    When executing query
      """
      match(n) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Person:Student {id:1}) |
      | (:Animal {id:2}) |
    When executing query
      """
      match(n:Person) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Person:Student {id:1}) |
    When executing query
      """
      match(n:Student) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Person:Student {id:1}) |
    When executing query
      """
      match(n:Musician) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Person:Student {id:1}) |
    When executing query
      """
      match(n:Animal) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Animal {id:2}) |
    When executing query
      """
      match(n {id:1}) remove n:Person;
      """
    When executing query
      """
      match(n {id:1}) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Student {id:1}) |
    When executing query
      """
      match(n {id:1}) remove n:Person;
      """
    When executing query
      """
      match(n {id:1}) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician:Student {id:1}) |
    When executing query
      """
      match(n {id:1}) remove n:Person:Student;
      """
    When executing query
      """
      match(n {id:1}) return n;
      """
    Then the result should be, in any order
      | n |
      | (:Musician {id:1}) |
    When executing query
      """
      match(n {id:1}) remove n:Person:Student:Musician;
      """
    When executing query
      """
      match(n {id:1}) return n;
      """
    Then the result should be, in any order
      | n |
      | ({id:1}) |
    When executing query
      """
      match(n:Person) return n;
      """
    Then the result should be empty
    When executing query
      """
      match(n:Student) return n;
      """
    Then the result should be empty
    When executing query
      """
      match(n:Musician) return n;
      """
    Then the result should be empty
    When executing query
      """
      match(n) set n:entity;
      """
    When executing query
      """
      match(n) return n;
      """
    Then the result should be, in any order
      | n |
      | (:entity {id:1}) |
      | (:Animal:entity{id:2}) |
    When executing query
      """
      match(n) remove n:entity;
      """
    When executing query
      """
      match(n) return n;
      """
    Then the result should be, in any order
      | n |
      | ({id:1}) |
      | (:Animal {id:2}) |