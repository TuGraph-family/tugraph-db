Feature: test expression
  Scenario: case01
    Given yago graph
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN n.birthyear, n.birthyear > 1900, n.birthyear > 2000;
      """
    Then the result should be, in any order
      | n.birthyear | n.birthyear > 1900 | n.birthyear > 2000 |
      | 1952 | true | false |
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN CASE WHEN n.birthyear < 1950 THEN 1 ELSE 2 END AS type /* 2 */;
      """
    Then the result should be, in any order
      | type |
      | 2 |
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN CASE WHEN n.birthyear < 1950 THEN 1 ELSE 2 END AS type1,CASE WHEN n.birthyear = 1952 THEN 1 ELSE 2 END AS type2 /* 2,1 */;
      """
    Then the result should be, in any order
      | type1 | type2 |
      | 2 | 1 |
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN CASE n.birthyear WHEN 1950 THEN 1 WHEN 1960 THEN 2 END AS type;
      """
    Then the result should be, in any order
      | type |
      | null |
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN CASE n.birthyear WHEN 1950 THEN 1 WHEN 1960 THEN 2 ELSE 3 END AS type;
      """
    Then the result should be, in any order
      | type |
      | 3 |
    When executing query
      """
      MATCH (n:Person {name:'Liam Neeson'}) RETURN CASE n.birthyear WHEN 1952 THEN 1 WHEN 1960 THEN 2 ELSE 3 END AS type;
      """
    Then the result should be, in any order
      | type |
      | 1 |
    When executing query
      """
      MATCH (n) RETURN CASE n.name WHEN null THEN false ELSE true END AS hasName;
      """
    Then the result should be, in any order
      | hasName |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | true |
      | false |
      | false |
      | false |
      | false |
      | false |
    When executing query
      """
      OPTIONAL MATCH (n {name:'Liam Neeson'}) RETURN CASE n WHEN null THEN false ELSE true END AS hasN;
      """
    Then the result should be, in any order
      | hasN |
      | true |
    When executing query
      """
      RETURN 2020;
      """
    Then the result should be, in any order
      | 2020 |
      | 2020 |
    When executing query
      """
      RETURN 1+2+3-4;
      """
    Then the result should be, in any order
      | 1+2+3-4 |
      | 2 |
    When executing query
      """
      RETURN 1+2 - (3+4);
      """
    Then the result should be, in any order
      | 1+2 - (3+4) |
      | -4 |
    When executing query
      """
      RETURN 1+2*3;
      """
    Then the result should be, in any order
      | 1+2*3 |
      | 7 |
    When executing query
      """
      RETURN (2+15)/2-3*8%10 /*4.5*/;
      """
    Then the result should be, in any order
      | (2+15)/2-3*8%10 |
      | 4 |
    When executing query
      """
      WITH 2 AS a,15 AS b RETURN (a+b)/a-a*b%4;
      """
    Then the result should be, in any order
      | (a+b)/a-a*b%4 |
      | 6 |
    When executing query
      """
      RETURN 2^3;
      """
    Then the result should be, in any order
      | 2^3 |
      | 8.0 |
    When executing query
      """
      RETURN 2^3^2;
      """
    Then the result should be, in any order
      | 2^3^2 |
      | 64.0 |
    When executing query
      """
      RETURN 2^(1+2)^2;
      """
    Then the result should be, in any order
      | 2^(1+2)^2 |
      | 64.0 |
    When executing query
      """
      RETURN 2^(1+2)*3^2-51/(8%5) /*55*/;
      """
    Then the result should be, in any order
      | 2^(1+2)*3^2-51/(8%5) |
      | 55.0 |
    When executing query
      """
      RETURN 1+2.0+3+4.0;
      """
    Then the result should be, in any order
      | 1+2.0+3+4.0 |
      | 10.0 |
    When executing query
      """
      RETURN 1+'a';
      """
    Then the result should be, in any order
      | 1+'a' |
      | '1a' |
    When executing query
      """
      RETURN ['a']+'a';
      """
    Then the result should be, in any order
      | ['a']+'a' |
      | ['a','a'] |
    When executing query
      """
      RETURN 1+[1];
      """
    Then the result should be, in any order
      | 1+[1] |
      | [1,1] |
    When executing query
      """
      RETURN TRUE+[1.0];
      """
    Then the result should be, in any order
      | TRUE+[1.0] |
      | [true,1.0] |
    When executing query
      """
      RETURN NULL+'a';
      """
    Then the result should be, in any order
      | NULL+'a' |
      | null |
    When executing query
      """
      RETURN 1+NULL
      """
    Then the result should be, in any order
      | 1+NULL |
      | null |
    When executing query
      """
      RETURN 1.0-2+3.0;
      """
    Then the result should be, in any order
      | 1.0-2+3.0 |
      | 2.0 |
    When executing query
      """
      RETURN NULL-1.1;
      """
    Then the result should be, in any order
      | NULL-1.1 |
      | null |
    When executing query
      """
      RETURN 1.0*2*3.0;
      """
    Then the result should be, in any order
      | 1.0*2*3.0 |
      | 6.0 |
    When executing query
      """
      RETURN 1.0*NULL;
      """
    Then the result should be, in any order
      | 1.0*NULL |
      | null |
    When executing query
      """
      RETURN 1.0/2/3.0;
      """
    Then the result should be, in any order
      | 1.0/2/3.0 |
      | 0.16666666666666666 |
    When executing query
      """
      RETURN 1.0/NULL;
      """
    Then the result should be, in any order
      | 1.0/NULL |
      | null |
    When executing query
      """
      RETURN 5%3;
      """
    Then the result should be, in any order
      | 5%3 |
      | 2 |
    When executing query
      """
      RETURN NULL%3;
      """
    Then the result should be, in any order
      | NULL%3 |
      | null |
    When executing query
      """
      RETURN 0^0;
      """
    Then the result should be, in any order
      | 0^0 |
      | 1.0 |
    When executing query
      """
      RETURN null^3.0;
      """
    Then the result should be, in any order
      | null^3.0 |
      | null |
    When executing query
      """
      RETURN 0^null;
      """
    Then the result should be, in any order
      | 0^null |
      | null |
    When executing query
      """
      RETURN (2.0+1)*3-2/1.5+'a'+[1];
      """
    Then the result should be, in any order
      | (2.0+1)*3-2/1.5+'a'+[1] |
      | ['7.666667a',1] |
    When executing query
      """
      RETURN null^2*3/4+5-6;
      """
    Then the result should be, in any order
      | null^2*3/4+5-6 |
      | null |
    When executing query
      """
      MATCH (n) WHERE n.name STARTS WITH 'Li' RETURN n,n.name;
      """
    Then the result should be, in any order
      | n | n.name |
      | (:Person{birthyear:1952,name:'LiamNeeson'}) | 'LiamNeeson' |
      | (:Person{birthyear:1986,name:'LindsayLohan'}) | 'LindsayLohan' |
    When executing query
      """
      MATCH (n) WHERE n.name ENDS WITH 'Redgrave' RETURN n,n.name
      """
    Then the result should be, in any order
      | n | n.name |
      | (:Person{birthyear:1908,name:'MichaelRedgrave'}) | 'MichaelRedgrave' |
      | (:Person{birthyear:1937,name:'VanessaRedgrave'}) | 'VanessaRedgrave' |
      | (:Person{birthyear:1939,name:'CorinRedgrave'}) | 'CorinRedgrave' |
      | (:Person{birthyear:1965,name:'JemmaRedgrave'}) | 'JemmaRedgrave' |
      | (:Person{birthyear:1873,name:'RoyRedgrave'}) | 'RoyRedgrave' |
    When executing query
      """
      MATCH (n) WHERE n.name CONTAINS 'Li' RETURN n,n.name
      """
    Then the result should be, in any order
      | n | n.name |
      | (:Person{birthyear:1952,name:'LiamNeeson'}) | 'LiamNeeson' |
      | (:Person{birthyear:1986,name:'LindsayLohan'}) | 'LindsayLohan' |
    When executing query
      """
      MATCH (n) WHERE n.name STARTS WITH 'Li&alonglongstring' RETURN n,n.name;
      """
    Then the result should be, in any order
      | n | n.name |
    When executing query
      """
      MATCH (n) WHERE n.name ENDS WITH 'on&alonglongstring' RETURN n,n.name;
      """
    Then the result should be, in any order
      | n | n.name |
    When executing query
      """
      MATCH (n) WHERE n.name CONTAINS 'on&alonglongstring' RETURN n,n.name;
      """
    Then the result should be, in any order
      | n | n.name |
    When executing query
      """
      MATCH (n) WHERE n.name REGEXP '.*' RETURN n.name;
      """
    Then the result should be, in any order
      | n.name |
      | 'RachelKempson'|
      | 'MichaelRedgrave'|
      | 'VanessaRedgrave'|
      | 'CorinRedgrave'|
      | 'LiamNeeson'|
      | 'NatashaRichardson'|
      | 'RichardHarris'|
      | 'DennisQuaid'|
      | 'LindsayLohan'|
      | 'JemmaRedgrave'|
      | 'RoyRedgrave'|
      | 'JohnWilliams'|
      | 'ChristopherNolan'|
      | 'NewYork'|
      | 'London'|
      | 'Houston'|
    When executing query
      """
      MATCH (n) WHERE n.name REGEXP 'Li.*' RETURN n.name
      """
    Then the result should be, in any order
      | n.name |
      | 'LiamNeeson'|
      | 'LindsayLohan'|
    When executing query
      """
      MATCH (n) WHERE n.name REGEXP '.*Redgrave' RETURN n.name
      """
    Then the result should be, in any order
      | n.name |
      | 'MichaelRedgrave'|
      | 'VanessaRedgrave'|
      | 'CorinRedgrave'|
      | 'JemmaRedgrave'|
      | 'RoyRedgrave'|
    When executing query
      """
      MATCH (n) WHERE n.name REGEXP '.*Redgrave' RETURN n.name
      """
    Then the result should be, in any order
      | n.name |
      | 'MichaelRedgrave'|
      | 'VanessaRedgrave'|
      | 'CorinRedgrave'|
      | 'JemmaRedgrave'|
      | 'RoyRedgrave'|
    When executing query
      """
      MATCH (n) WHERE n.name REGEXP '.*ee.*' RETURN n.name;
      """
    Then the result should be, in any order
      | n.name |
      | 'LiamNeeson'|
    When executing query
      """
      MATCH p = (n {name:'Rachel Kempson'})-[]->() RETURN p;
      """
    Then the result should be, in any order
      | p |
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})>|
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})>  |
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})>  |

    When executing query
      """
      MATCH p = (n {name:'Rachel Kempson'})<-[]-() RETURN p;
      """
    Then the result should be, in any order
      | p |
      | <(:Person{birthyear:1910,name:'RachelKempson'})<-[:MARRIED]-(:Person{birthyear:1908,name:'MichaelRedgrave'})>|

    When executing query
      """
      MATCH p = (n {name:'Rachel Kempson'})-[]-() RETURN p
      """
    Then the result should be, in any order
      | p |
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})>|
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})> |
      | <(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})> |
      | <(:Person{birthyear:1910,name:'RachelKempson'})<-[:MARRIED]-(:Person{birthyear:1908,name:'MichaelRedgrave'})>  |

    When executing query
      """
      MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]->() RETURN p
      """
    Then the result should be, in any order
      | p |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1963,name:'NatashaRichardson'})>|
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:BORN_IN{weight:20.21}]->(:City{name:'London'})>                |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:ACTED_IN{charactername:'Guenevere'}]->(:Film{title:'Camelot'})>|
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1965,name:'JemmaRedgrave'})>     |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})>    |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})>      |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:MARRIED]->(:Person{birthyear:1910,name:'RachelKempson'})>        |
    |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:ACTED_IN{charactername:'TheHeadmaster'}]->(:Film{title:'GoodbyeMr.Chips'})>|

    When executing query
      """
      MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]->(m) RETURN p,m;
      """
    Then the result should be, in any order
      | p | m|
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1963,name:'NatashaRichardson'})>|(:Person{birthyear:1963,name:'NatashaRichardson'})|
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:BORN_IN{weight:20.21}]->(:City{name:'London'})>                |(:City{name:'London'})                            |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})-[:ACTED_IN{charactername:'Guenevere'}]->(:Film{title:'Camelot'})>|(:Film{title:'Camelot'})                          |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1965,name:'JemmaRedgrave'})>     |(:Person{birthyear:1965,name:'JemmaRedgrave'})     |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1937,name:'VanessaRedgrave'})>    |(:Person{birthyear:1937,name:'VanessaRedgrave'})  |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:HAS_CHILD]->(:Person{birthyear:1939,name:'CorinRedgrave'})>      |(:Person{birthyear:1939,name:'CorinRedgrave'})    |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:MARRIED]->(:Person{birthyear:1910,name:'RachelKempson'})>        |(:Person{birthyear:1910,name:'RachelKempson'})    |
      |<(:Person{birthyear:1910,name:'RachelKempson'})-[:MARRIED]->(:Person{birthyear:1908,name:'MichaelRedgrave'})-[:ACTED_IN{charactername:'TheHeadmaster'}]->(:Film{title:'GoodbyeMr.Chips'})>|(:Film{title:'GoodbyeMr.Chips'})        |
