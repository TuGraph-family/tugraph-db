MATCH (m:movie {title:'Forrest Gump'})<-[r:rate]-(u:user)-[r2:rate]->(m2:movie) WHERE r.stars>3 AND r2.stars>3 RETURN m, u,m2;
