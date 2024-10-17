(async () => {
    var neo4j = require('neo4j-driver');
    var driver = neo4j.driver('bolt://localhost:7687', neo4j.auth.basic('admin', '73@TuGraph'));
    var session = driver.session({database: 'default'})
    await session.run("CALL db.dropDB()");
    console.log("clean db");
    await session.run("CALL db.createVertexLabel('person', 'id' , 'id', 'INT32', false, 'name', 'STRING', false)");
    console.log("add vertex label");
    await session.run("CALL db.createEdgeLabel('is_friend','[[\"person\",\"person\"]]')");
    console.log("add edge label");
    await session.run("create (n1:person {name:'jack',id:1}), (n2:person {name:'lucy',id:2})")
    console.log("create two nodes");
    await session.run("match (n1:person {id:1}), (n2:person {id:2}) create (n1)-[r:is_friend]->(n2)")
    console.log("create edge");
    var res = await session.run("match (n)-[r]->(m) return n,r,m")
    for (var i=0; i < res.records.length; i++) {
        console.log(res.records[i].get("n"))
        console.log(res.records[i].get("r"))
        console.log(res.records[i].get("m"))
    }
    await session.close();
    await driver.close();
})();
