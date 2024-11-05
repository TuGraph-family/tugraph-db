Feature: test hint
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (rachel:Person {name:'Rachel Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person {name:'Richard Harris'}) RETURN family.name;
      '''
    Then the result should be, in any order
      | family.name      |
      | 'Vanessa Redgrave' |
    When executing query
      '''
      MATCH (rachel:Person {name:'Rachel Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person {name:'Richard Harris'}) USING JOIN ON film RETURN family.name;
      '''
    Then the result should be, in any order
      | family.name      |
      | 'Vanessa Redgrave' |
    When executing query
      '''
      MATCH (rachel:Person {name:'Rachel Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person {name:'Richard Harris'}) USING JOIN ON family RETURN family.name;
      '''
    Then the result should be, in any order
      | family.name      |
      | 'Vanessa Redgrave' |
    When executing query
      '''
      MATCH (camelot:Film {title:'Camelot'})<-[:ACTED_IN]-(actor)-[]->(x) USING START ON camelot RETURN x;
      '''
    Then the result should be, in any order
      | x                                                       |
      | (:Person {name:'Natasha Richardson',birthyear:1963})    |
      | (:City {name:'London'})                                 |
      | (:Film {title:'Harry Potter and the Sorcerer's Stone'}) |