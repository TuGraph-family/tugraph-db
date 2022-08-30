//PASSED:
//MATCH (tom:Person {name: "Tom Hanks"})-[:ACTED_IN]->(tomHanksMovies) RETURN tom,tomHanksMovies
//MATCH (na)-[]->(nb)-[]->(nc) RETURN na,nb,nc
//MATCH (na:Person)-[]->(nb)-[]->(nc) RETURN na,nb,nc
//MATCH (na:Person)-[]->(nb)-[:FOLLOWS]->(nc) RETURN na,nb,nc
/* todo: user specified entry node by START clause */
//MATCH (cloudAtlas {title: "Cloud Atlas"})<-[:DIRECTED]-(directors) RETURN directors.name
//MATCH (na)<-[]-(nb {name: "Angela Scope"})<-[]-(nc) RETURN na,nb,nc
//MATCH (tom:Person {name:"Tom Hanks"})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors) RETURN coActors.name
//MATCH (tom:Person {name:"Tom Hanks"})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors:Person {born:1967}) RETURN m,coActors.name
//MATCH (tom:Person {name:"Tom Hanks"})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors) RETURN DISTINCT coActors.name
//MATCH (n)-[relatedTo]->(jessica:Person {name:"Jessica Thompson"}) RETURN n,relatedTo
//MATCH (n)<-[relatedTo]-(jessica:Person {name:"Jessica Thompson"}) RETURN n.title,relatedTo.rating
//MATCH (jessica:Person {name:"Jessica Thompson"})-[relatedTo]-(n) RETURN n,relatedTo
//MATCH (n)-[relatedTo]-(jessica:Person {name:"Jessica Thompson"}) RETURN n,relatedTo
//MATCH (people:Person)-[relatedTo]-(:Movie {title: "Cloud Atlas"}) RETURN people.name
//MATCH (people:Person)-[relatedTo]-(:Movie {title: "Cloud Atlas"}) RETURN people.name, relatedTo

//TODO:
//MATCH (n:Person {name:'Vanessa Redgrave'})-[:ACTED_IN|:BORN_IN]->(m) RETURN m
//MATCH (people:Person)-[relatedTo]-(:Movie {title: "Cloud Atlas"}) RETURN people.name, Type(relatedTo), relatedTo
