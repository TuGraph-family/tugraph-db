MATCH (m:movie {title: 'Forrest Gump'})<-[r:acted_in]-(a:person) RETURN a.name,r.role;
