MATCH (m:movie {title: 'Forrest Gump'})<-[:acted_in]-(a:person) RETURN a, m;
