Feature: test list comprehension
  Scenario: case01
    Given an empty graph
    When executing query
      '''
      RETURN [x IN range(0,10) | x] AS result;
      '''
    Then the result should be, in any order
      | result                   |
      | [0,1,2,3,4,5,6,7,8,9,10] |
    When executing query
      '''
      RETURN [x IN range(0,10) | x^3] AS result;
      '''
    Then the result should be, in any order
      | result                                                       |
      | [0.0,1.0,8.0,27.0,64.0,125.0,216.0,343.0,512.0,729.0,1000.0] |
    When executing query
      '''
      WITH [2,4,6] AS y RETURN [x IN y | x] AS result;
      '''
    Then the result should be, in any order
      | result  |
      | [2,4,6] |
    When executing query
      '''
      WITH [2,4,6] AS y RETURN [x IN range(0, size(y)) | x] AS result;
      '''
    Then the result should be, in any order
      | result    |
      | [0,1,2,3] |
    When executing query
      '''
      RETURN [x in [x in [x in [1,2] |x+1] |x+1] | x+1+head([x in [1,2,3] | x^2])];
      '''
    Then the result should be, in any order
      | [x in [x in [x in [1,2] \|x+1] \|x+1] \| x+1+head([x in [1,2,3] \| x^2])] |
      | [5.0,6.0]                                                             |
    When executing query
      '''
      RETURN [x in [1] | x + head([x in [2,3] | x+1]) +  x ];
      '''
    Then the result should be, in any order
      | [x in [1] \| x + head([x in [2,3] \| x+1]) +  x ] |
      | [5]                                             |
    When executing query
      '''
      RETURN [x in [x in [x in [2,3,4] |x+1] |x+1] | x+1+head([x in [1,2,3] | x^2])+x];
      '''
    Then the result should be, in any order
      | [x in [x in [x in [2,3,4] \|x+1] \|x+1] \| x+1+head([x in [1,2,3] \| x^2])+x] |
      | [10.0,12.0,14.0]                                                          |
    When executing query
      '''
      RETURN [x in [1] | x + head([x]) + x];
      '''
    Then the result should be, in any order
      | [x in [1] \| x + head([x]) + x] |
      | [3]                            |
    When executing query
      '''
      RETURN [x in [1] | x + head([x in [2] | x]) + x];
      '''
    Then the result should be, in any order
      | [x in [1] \| x + head([x in [2] \| x]) + x] |
      | [4]                                       |
    When executing query
      '''
      RETURN [ _ in range(1,10) | x];
      '''
    Then an Error should be raised