Feature: test function
  Scenario: case01
    Given yago graph
    When executing query
      '''
      MATCH (n:Person) RETURN properties(n) LIMIT 2;
      '''
    Then the result should be, in any order
      | properties(n)                                |
      | {birthyear:1910,name:'Rachel Kempson'}   |
      | {birthyear:1908,name:'Michael Redgrave'} |
    When executing query
      '''
      MATCH p=(n:Person)-[e*..2]->(m) RETURN properties(p) LIMIT 2;
      '''
    Then the result should be, in any order
      | properties(p)                                                                                |
      | [{birthyear:1910,name:'Rachel Kempson'},{},{birthyear:1937,name:'Vanessa Redgrave'}] |
      | [{birthyear:1910,name:'Rachel Kempson'},{},{birthyear:1939,name:'Corin Redgrave'}]   |
    When executing query
      '''
      MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[relatedTo]-(n) RETURN id(vanessa),type(relatedTo),labels(n);
      '''
    Then the result should be, in any order
      | id(vanessa) | type(relatedTo) | labels(n)  |
      | 3           | 'HAS_CHILD'       | ['Person'] |
      | 3           | 'BORN_IN'         | ['City']   |
      | 3           | 'ACTED_IN'        | ['Film']   |
      | 3           | 'HAS_CHILD'       | ['Person'] |
      | 3           | 'HAS_CHILD'       | ['Person'] |
    When executing query
      '''
      MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->() RETURN startNode(r),endNode(r);
      '''
    Then the result should be, in any order
      | startNode(r) | endNode(r) |
      | 3            | 6          |
      | 3            | 15         |
      | 3            | 21         |
    When executing query
      '''
      MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->(n) RETURN properties(n);
      '''
    Then the result should be, in any order
      | properties(n)                                  |
      | {birthyear:1963,name:'Natasha Richardson'} |
      | {name:'London'}                              |
      | {title:'Camelot'}                            |
    When executing query
      '''
      MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->(n) RETURN properties(r);
      '''
    Then the result should be, in any order
      | properties(r)                 |
      | {}                            |
      | {weight:20.21}              |
      | {charactername:'Guenevere'} |
    When executing query
      '''
      MATCH (a) WHERE a.name = 'Vanessa Redgrave' RETURN labels(a);
      '''
    Then the result should be, in any order
      | labels(a)  |
      | ['Person'] |
    When executing query
      '''
      MATCH (a) WHERE a.name = 'Vanessa Redgrave' RETURN keys(a);
      '''
    Then the result should be, in any order
      | keys(a)              |
      | ['name','birthyear'] |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'}) RETURN a,-2,9.78,'im a string';
      '''
    Then the result should be, in any order
      | a                                                  | -2 | 9.78 | 'im a string' |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) | -2 | 9.78 | 'im a string'   |
    When executing query
      '''
      MATCH (a:Person {name:'Vanessa Redgrave'}) RETURN a,abs(-2),ceil(0.1),floor(0.9),tointeger(rand()),round(3.141592),sign(-17),sign(0.1);
      '''
    Then the result should be, in any order
      | a                                                  | abs(-2) | ceil(0.1) | floor(0.9) | tointeger(rand()) | round(3.141592) | sign(-17) | sign(0.1) |
      | (:Person {name:'Vanessa Redgrave',birthyear:1937}) | 2       | 1.0       | 0.0        | 0                 | 3.0             | -1        | 1         |
    When executing query
      '''
      RETURN toInteger(2.0),toInteger(2.3),toInteger('3');
      '''
    Then the result should be, in any order
      | toInteger(2.0) | toInteger(2.3) | toInteger('3') |
      | 2              | 2              | 3              |
    When executing query
      '''
      RETURN toBoolean(true),toBoolean('True');
      '''
    Then the result should be, in any order
      | toBoolean(true) | toBoolean('True') |
      | true            | true              |
    When executing query
      '''
      RETURN toFloat(2),toFloat(2.3),toFloat('3'),toFloat('2.019');
      '''
    Then the result should be, in any order
      | toFloat(2) | toFloat(2.3) | toFloat('3') | toFloat('2.019') |
      | 2.0        | 2.3          | 3.0          | 2.019            |
    When executing query
      '''
      RETURN toString(2),toString(2.3),toString(true),toString('haha');
      '''
    Then the result should be, in any order
      | toString(2) | toString(2.3) | toString(true) | toString('haha') |
      | '2'           | '2.300000'      | 'true'           | 'haha'             |
    When executing query
      '''
      RETURN size('hello world!');
      '''
    Then the result should be, in any order
      | size('hello world!') |
      | 12                   |
    When executing query
      '''
      MATCH (n:Person) WHERE size(n.name) > 15 RETURN n.name,size(n.name);
      '''
    Then the result should be, in any order
      | n.name             | size(n.name) |
      | 'Michael Redgrave'   | 16           |
      | 'Vanessa Redgrave'   | 16           |
      | 'Natasha Richardson' | 18           |
      | 'Christopher Nolan'  | 17           |
    When executing query
      '''
      WITH ['one','two','three'] AS coll RETURN size(coll);
      '''
    Then the result should be, in any order
      | size(coll) |
      | 3          |
    When executing query
      '''
      WITH ['one','two','three'] AS coll RETURN head(coll);
      '''
    Then the result should be, in any order
      | head(coll) |
      | 'one'        |
    When executing query
      '''
      WITH ['one','two','three'] AS coll RETURN last(coll);
      '''
    Then the result should be, in any order
      | last(coll) |
      | 'three'      |
    When executing query
      '''
      WITH ['one','two','three'] AS coll UNWIND coll AS x RETURN collect(x);
      '''
    Then the result should be, in any order
      | collect(x)            |
      | ['one','two','three'] |
    When executing query
      '''
      WITH ['one','two','three'] AS coll UNWIND coll AS x WITH collect(x) AS reColl RETURN head(reColl);
      '''
    Then the result should be, in any order
      | head(reColl) |
      | 'one'          |
    When executing query
      '''
      MATCH (n:Person) RETURN sum(n.birthyear);
      '''
    Then the result should be, in any order
      | sum(n.birthyear) |
      | 25219.0          |
    When executing query
      '''
      MATCH (n:Person) RETURN labels(n),sum(n.birthyear);
      '''
    Then the result should be, in any order
      | labels(n)  | sum(n.birthyear) |
      | ['Person'] | 25219.0          |
    When executing query
      '''
      MATCH (n:Person) RETURN n.name,sum(n.birthyear);
      '''
    Then the result should be, in any order
      | n.name               | sum(n.birthyear) |
      | 'John Williams'      | 1932.0           |
      | 'Roy Redgrave'       | 1873.0           |
      | 'Christopher Nolan'  | 1970.0           |
      | 'Richard Harris'     | 1930.0           |
      | 'Natasha Richardson' | 1963.0           |
      | 'Corin Redgrave'     | 1939.0           |
      | 'Vanessa Redgrave'   | 1937.0           |
      | 'Dennis Quaid'       | 1954.0           |
      | 'Liam Neeson'        | 1952.0           |
      | 'Michael Redgrave'   | 1908.0           |
      | 'Jemma Redgrave'     | 1965.0           |
      | 'Lindsay Lohan'      | 1986.0           |
      | 'Rachel Kempson'     | 1910.0           |
    When executing query
      '''
      MATCH (n:Person) RETURN n.name,labels(n),sum(n.birthyear);
      '''
    Then the result should be, in any order
      | n.name             | labels(n)  | sum(n.birthyear) |
      | 'Roy Redgrave'       | ['Person'] | 1873.0           |
      | 'Jemma Redgrave'     | ['Person'] | 1965.0           |
      | 'Dennis Quaid'       | ['Person'] | 1954.0           |
      | 'John Williams'      | ['Person'] | 1932.0           |
      | 'Lindsay Lohan'      | ['Person'] | 1986.0           |
      | 'Richard Harris'     | ['Person'] | 1930.0           |
      | 'Natasha Richardson' | ['Person'] | 1963.0           |
      | 'Liam Neeson'        | ['Person'] | 1952.0           |
      | 'Christopher Nolan'  | ['Person'] | 1970.0           |
      | 'Corin Redgrave'     | ['Person'] | 1939.0           |
      | 'Vanessa Redgrave'   | ['Person'] | 1937.0           |
      | 'Michael Redgrave'   | ['Person'] | 1908.0           |
      | 'Rachel Kempson'     | ['Person'] | 1910.0           |
    When executing query
      '''
      MATCH (n:Person {name:'Natasha Richardson'})--(m:Person) RETURN m.name,sum(m.birthyear);
      '''
    Then the result should be, in any order
      | m.name           | sum(m.birthyear) |
      | 'Vanessa Redgrave' | 1937.0           |
      | 'Liam Neeson'      | 3904.0           |
    When executing query
      '''
      MATCH (n:Person) RETURN count(n);
      '''
    Then the result should be, in any order
      | count(n) |
      | 13       |
    When executing query
      '''
      MATCH (n:Person) RETURN avg(n.birthyear);
      '''
    Then the result should be, in any order
      | avg(n.birthyear)  |
      | 1939.923076923077 |
    When executing query
      '''
      MATCH (n:Person) RETURN max(n.birthyear),min(n.birthyear),sum(n.birthyear);
      '''
    Then the result should be, in any order
      | max(n.birthyear) | min(n.birthyear) | sum(n.birthyear) |
      | 1986.0           | 1873.0           | 25219.0          |
    When executing query
      '''
      OPTIONAL MATCH (n:City {name:'London'})-[r]->() RETURN count(r);
      '''
    Then the result should be, in any order
      | count(r) |
      | 0        |
    When executing query
      '''
      OPTIONAL MATCH (n:City {name:'London'})-[r]->() RETURN count(*);
      '''
    Then the result should be, in any order
      | count(*) |
      | 1        |
    When executing query
      '''
      MATCH (n:Person) RETURN count(n) AS num_person;
      '''
    Then the result should be, in any order
      | num_person |
      | 13         |
    When executing query
      '''
      match (city:City {name:'New York'}) return id(city) as cityId, coalesce(city.name, city.cname) as cityName;
      '''
    Then the result should be, in any order
      | cityId | cityName |
      | 14     | 'New York' |
    When executing query
      '''
      RETURN coalesce(null);
      '''
    Then the result should be, in any order
      | coalesce(null) |
      | null           |
    When executing query
      '''
      RETURN coalesce(2021);
      '''
    Then the result should be, in any order
      | coalesce(2021) |
      | 2021           |
    When executing query
      '''
      RETURN coalesce(2021, null);
      '''
    Then the result should be, in any order
      | coalesce(2021, null) |
      | 2021                 |
    When executing query
      '''
      RETURN coalesce(null, null);
      '''
    Then the result should be, in any order
      | coalesce(null, null) |
      | null                 |
    When executing query
      '''
      MATCH (n) RETURN coalesce(n.birthyear, n.name);
      '''
    Then the result should be, in any order
      | coalesce(n.birthyear, n.name) |
      | 1910                          |
      | 1908                          |
      | 1937                          |
      | 1939                          |
      | 1952                          |
      | 1963                          |
      | 1930                          |
      | 1954                          |
      | 1986                          |
      | 1965                          |
      | 1873                          |
      | 1932                          |
      | 1970                          |
      | 'New York'                      |
      | 'London'                        |
      | 'Houston'                       |
      | null                          |
      | null                          |
      | null                          |
      | null                          |
      | null                          |
    When executing query
      '''
      RETURN abs('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN ceil('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN floor('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN round('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN sign('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN toboolean('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN tofloat('haha');
      '''
    Then an Error should be raised
    When executing query
      '''
      RETURN tointeger('haha');
      '''
    Then an Error should be raised