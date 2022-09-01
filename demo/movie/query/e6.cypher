MATCH (u:user {login: 'Michael'})-[:is_friend]->(v:user)-[r:rate]->(m:movie) WHERE r.stars > 3 RETURN u, v, m;
