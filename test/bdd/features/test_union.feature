Feature: test union
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n:Person)-[:BORN_IN]->(:City {name:'London'}) RETURN n.name UNION MATCH (n:Person)-[:ACTED_IN]->(:Film {title:'The Parent Trap'}) RETURN n.name;
      '''
    Then the result should be, in any order
      | n.name               |
      | 'Vanessa Redgrave  ' |
      | 'Natasha Richardson' |
      | 'Christopher Nolan ' |
      | 'Natasha Richardson' |
      | 'Dennis Quaid      ' |
      | 'Lindsay Lohan     ' |
    When executing query
      '''
      MATCH (n:Person) RETURN n.name AS name UNION MATCH (m:Film) RETURN m.title AS name;
      '''
    Then the result should be, in any order
      | name                                   |
      | 'Rachel Kempson                      ' |
      | 'Michael Redgrave                    ' |
      | 'Vanessa Redgrave                    ' |
      | 'Corin Redgrave                      ' |
      | 'Liam Neeson                         ' |
      | 'Natasha Richardson                  ' |
      | 'Richard Harris                      ' |
      | 'Dennis Quaid                        ' |
      | 'Lindsay Lohan                       ' |
      | 'Jemma Redgrave                      ' |
      | 'Roy Redgrave                        ' |
      | 'John Williams                       ' |
      | 'Christopher Nolan                   ' |
      | 'Goodbye Mr. Chips                   ' |
      | 'Batman Begins                       ' |
      | 'Harry Potter and the Sorcerer's Stone' |
      | 'The Parent Trap                     ' |
      | 'Camelot                             ' |
    When executing query
      '''
      MATCH (n:Person)-[:BORN_IN]->(:City {name:'London'}) RETURN n.name UNION MATCH (n:Person)-[:ACTED_IN]->(:Film {title:'The Parent Trap'}) RETURN n.age;
      '''
    Then an Error should be raised
