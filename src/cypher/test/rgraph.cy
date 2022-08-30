//MATCH (n) RETURN n
MATCH (n:Movie) RETURN n
//MATCH (a:person)-[r1:lives]->(city)<-[r2]-(b:person)<-[]-(c)-[:owns]->() return city.name
/* redis-graph query
CREATE (aldis:actor {name: "Aldis Hodge", birth_year: 1986}),
       (oshea:actor {name: "OShea Jackson", birth_year: 1991}),
       (corey:actor {name: "Corey Hawkins", birth_year: 1988}),
       (neil:actor {name: "Neil Brown", birth_year: 1980}),
       (passer:actor {name: "Passerby A", birth_year: 1989}),
       (compton:movie {title: "Straight Outta Compton", genre: "Biography", votes: 127258, rating: 7.9, year: 2015}),
       (neveregoback:movie {title: "Never Go Back", genre: "Action", votes: 15821, rating: 6.4, year: 2016}),
       (aldis)-[:act]->(neveregoback),
       (aldis)-[:act]->(compton),
       (oshea)-[:act]->(compton),
       (corey)-[:act]->(compton),
       (neil)-[:act]->(compton),
       (oshea)-[:knows]->(aldis),
       (passer)-[:knows]->(corey),
       (passer)-[:act]->(neveregoback)
*/
//MATCH (neil:actor {name:"Neil Brown"})-[act]->(movie) RETURN movie
//MATCH (oshea:actor {name:"OShea Jackson"})-[:act]->(m:movie)<-[r]-(a:actor) RETURN m.title, a.name
//MATCH (oshea:actor {name:"OShea Jackson"})-[:act]->(m:movie)<-[r]-(a:actor) WHERE a.birth_year >= 1988 RETURN m.title, a.name
//MATCH (neil:actor {name:"Neil Brown"})-[act]->(movie)<-[:act]-(coActor)<-[knows]-(personA) WHERE coActor.birth_year > 1987 AND personA.birth_year > 1988 RETURN movie,coActor,personA
