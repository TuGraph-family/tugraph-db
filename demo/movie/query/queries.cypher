# who acted in Forrest Gump
MATCH (m:movie {title: 'Forrest Gump'})<-[:acted_in]-(a:person) RETURN a, m

# who played who in Forrest Gump
MATCH (m:movie {title: 'Forrest Gump'})<-[r:acted_in]-(a:person) RETURN a.name,r.role

# which movies does Michael hate
MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie) WHERE r.stars < 3 RETURN m.title, r.stars

# who hate the movies I hate
MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie)<-[s:rate]-(v) WHERE r.stars < 3 AND s.stars < 3 RETURN u, m, v

# what do those with my taste like?
MATCH (u:user {login: 'Michael'})-[r:rate]->(m:movie)<-[s:rate]-(v)-[r2:rate]->(m2:movie) WHERE r.stars < 3 AND s.stars < 3 AND r2.stars > 3 RETURN u, m, v, m2

# what do my friends like?
MATCH (u:user {login: 'Michael'})-[:is_friend]->(v:user)-[r:rate]->(m:movie) WHERE r.stars > 3 RETURN u, v, m

# those who liked Forrest Gump also like...
MATCH (m:movie {title:'Forrest Gump'})<-[r:rate]-(u:user)-[r2:rate]->(m2:movie) WHERE r.stars>3 AND r2.stars>3 RETURN m, u,m2
