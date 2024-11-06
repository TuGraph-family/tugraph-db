from neo4j import GraphDatabase
URI = "bolt://localhost:7687"
AUTH = ("admin", "73@TuGraph")
with GraphDatabase.driver(URI, auth=AUTH) as client:
    session = client.session(database="default")
    session.run("CREATE (n:person {id:1, name: 'James'})")
    session.run("CREATE (n:person {id:2, name: 'Robert'})")
    session.run("MATCH (n1:person {id:1}), (n2:person {id:2}) CREATE (n1)-[r:is_friend]->(n2)")
    ret = session.run("match (n)-[r]->(m) return n,r,m")
    for item in ret.data():
        print(item)