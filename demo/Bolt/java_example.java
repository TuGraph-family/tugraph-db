package org.example;
import org.neo4j.driver.*;
import org.neo4j.driver.Record;
import org.neo4j.driver.types.Node;
import org.neo4j.driver.types.Relationship;
import java.util.List;

public class Example {
    public static void main( final String[] args ) {
        Driver driver = GraphDatabase.driver("bolt://localhost:7687", AuthTokens.basic("admin", "73@TuGraph"));
        Session session = driver.session(SessionConfig.forDatabase("default"));
        session.run("CALL db.dropDB()");
        session.run("CALL db.createVertexLabel('person', 'id' , 'id' ,'INT32', false, 'name', 'STRING', false)");
        session.run("CALL db.createEdgeLabel('is_friend','[[\"person\",\"person\"]]')");
        session.run("create (n1:person {name:'jack',id:1}), (n2:person {name:'lucy',id:2})");
        session.run("match (n1:person {id:1}), (n2:person {id:2}) create (n1)-[r:is_friend]->(n2)");
        Result res = session.run("match (n)-[r]->(m) return n,r,m");
        List<Record> records =  res.list();
        for (Record record : records) {
            Node n = record.get("n").asNode();
            System.out.println(n.asMap());
            Relationship r = record.get("r").asRelationship();
            System.out.println(r.asMap());
            Node m = record.get("m").asNode();
            System.out.println(m.asMap());
        }
    }
}
