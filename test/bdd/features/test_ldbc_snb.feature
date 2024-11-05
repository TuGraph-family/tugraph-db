Feature: test ldbc snb
  Scenario: case01
    Given an empty graph
    When executing query
      '''
      CREATE   (tagClass1:TagClass {name: 'MusicalArtist'}),   (tagClass2:TagClass {name: 'OfficeHolder'}),   (tag1:Tag {name: 'Elvis Presley'}),   (tag2:Tag {name: 'Mr. Office'}),   (forum1:Forum {name: 'Presley fan club'}),   (forum2:Forum {name: 'Long live the closed office'}),   (forum1)-[:hasTag]->(tag1)-[:hasType]->(tagClass1),   (forum2)-[:hasTag]->(tag2)-[:hasType]->(tagClass2),   (person1:Person {id: 1, birthday: '1990-01-01'}),   (person2:Person {id: 2, birthday: '1989-01-01'}),   (person3:Person {id: 3, birthday: '1989-01-01'}),   (person4:Person {id: 4, birthday: '1989-01-01'}),   (person1)-[:knows]->(person2),   (person2)-[:knows]->(person3),   (person3)-[:knows]->(person4),   (forum1)-[:hasMember]->(person3)<-[:hasMember]-(forum2),   (forum1)-[:hasMember]->(person4)<-[:hasMember]-(forum2),   (comment1:Comment {id: 1}),   (comment2:Comment {id: 2}),   (comment3:Comment {id: 3}),   (message1:Message {id: 1}),   (message2:Message {id: 2}),   (message3:Message {id: 3}),   (person1)<-[:hasCreator]-(comment1)-[:replyOf]->(message1)-[:hasCreator]->(person3),   (person3)<-[:hasCreator]-(comment2)-[:replyOf]->(message2)-[:hasCreator]->(person1),   (person4)<-[:hasCreator]-(comment3)-[:replyOf]->(message3)-[:hasCreator]->(person1);
      '''
    Then the result should be, in any order
      | <SUMMARY>                              |
      | 'created 16 vertices, created 20 edges.' |