// PASSED:
//MATCH n RETURN n
//MATCH n RETURN n.name,n.title
//MATCH n:Movie RETURN n.title
//MATCH (n:Movie) WHERE n.released > 2000 RETURN n.title,n.released
//MATCH (tom:Person {name: "Tom Hanks"}) RETURN tom
//MATCH (n:Person {born: 1974}) RETURN n.name,n.born
//MATCH (cloudAtlas:Movie {title: "Cloud Atlas"}) RETURN cloudAtlas
//MATCH (people:Person) RETURN people.name LIMIT 10
//MATCH (people) RETURN people.name LIMIT 10
//MATCH (nineties:Movie) WHERE nineties.released >= 1990 AND nineties.released < 2000 RETURN nineties.title
