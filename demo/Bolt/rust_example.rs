use neo4rs::*;

// https://github.com/neo4j-labs/neo4rs
#[tokio::main]
async fn main() {
    let config = ConfigBuilder::default()
        .uri("bolt://100.88.118.28:19090")
        .user("admin")
        .password("73@TuGraph")
        .db("default")
        .build()
        .unwrap();

    let graph = Graph::connect(config).await.unwrap();
    {
        graph.run(query("CALL db.dropDB()")).await.unwrap();
        graph.run(query("CALL db.createVertexLabel('person', 'id' , 'id', 'INT32', false, 'name', 'STRING', false)")).await.unwrap();
        graph.run(query("CALL db.createEdgeLabel('is_friend','[[\"person\",\"person\"]]')")).await.unwrap();
        graph.run(query("create (n1:person {name:'jack',id:1}), (n2:person {name:'lucy',id:2})")).await.unwrap();
        graph.run(query("match (n1:person {id:1}), (n2:person {id:2}) create (n1)-[r:is_friend]->(n2)")).await.unwrap();
        let mut result = graph.execute(query("match (n)-[r]->(m) return n,r,m")).await.unwrap();
        while let Ok(Some(row)) = result.next().await {
            let n: Node = row.get("n").unwrap();
            let r: Relation = row.get("r").unwrap();
            let m: Node = row.get("m").unwrap();
            println!("n: [{}]", n.id());
            println!("r: [{}->{}]", r.start_node_id(), r.end_node_id());
            println!("m: [{}]", m.id());
        }
    }
}
