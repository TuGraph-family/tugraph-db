from neo4j import GraphDatabase

URI = "bolt://localhost:7687"
AUTH = ("admin", "73@TuGraph")
with GraphDatabase.driver(URI, auth=AUTH) as client:
    session = client.session(database="default")
    session.run("CALL db.dropDB()")
    session.run("CALL db.createVertexLabel('person', 'id' , 'id', 'INT32', false, 'name', 'STRING', false)")
    session.run("CALL db.createEdgeLabel('is_friend','[[\"person\",\"person\"]]')")
    session.run("create (n1:person {name:'jack',id:1}), (n2:person {name:'lucy',id:2})")
    session.run("match (n1:person {id:1}), (n2:person {id:2}) create (n1)-[r:is_friend]->(n2)")
    ret = session.run("match (n)-[r]->(m) return n,r,m")
    for item in ret.data():
        print(item)
