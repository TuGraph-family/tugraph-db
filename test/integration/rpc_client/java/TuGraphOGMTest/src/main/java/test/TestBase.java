package test;

import com.alibaba.fastjson.JSONObject;
import entity.Actor;
import entity.Movie;
import org.neo4j.ogm.cypher.ComparisonOperator;
import org.neo4j.ogm.model.QueryStatistics;
import org.neo4j.ogm.model.Result;
import org.neo4j.ogm.session.Session;
import org.neo4j.ogm.session.SessionFactory;
import org.neo4j.ogm.cypher.Filter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.*;

import static java.util.Collections.emptyMap;
import static org.assertj.core.api.Assertions.*;


public class TestBase extends Client{
    private static final Logger log = LoggerFactory.getLogger(TestBase.class);
    private static SessionFactory sessionFactory;
    private static Session session;


    public static void main(String[] args) {
        if (args.length != 3) {
            log.info("java -jar target/TuGpraphOgmTest-1.0-SNAPSHOT.jar [host:port] [user] [password]");
            return;
        }
        sessionFactory = new SessionFactory(getDriver(args), "entity");
        session = sessionFactory.openSession();

        testCreate();
        testQuery();
        testUpdate();
        testDelete();
    }

    private static void testDelete() {
        log.info("----------------testDelete--------------------");
        // Test1  CREATE -> DELETE
        Actor a1 = new Actor();
        Actor a2 = new Actor();
        a1.setName("ado");
        a2.setName("abo");
        session.save(a1);
        session.save(a2);
        Collection<Actor> savedActors = session.loadAll(Actor.class);
        assertThat(savedActors).hasSize(4);

        List<Object> actorList = new ArrayList<>();
        actorList.add(a1);
        actorList.add(a2);
        session.delete(actorList);
        Collection<Actor> delActors = session.loadAll(Actor.class);
        assertThat(delActors).hasSize(2);

        // Test2  MATCH -> DELETE
        Result result = session.query("MATCH (n)-[r]->(n2) DELETE r", emptyMap());
        QueryStatistics statistics = result.queryStatistics();
        log.info("deleted " + statistics.getRelationshipsDeleted() + " edges");
        assertThat(statistics.getRelationshipsDeleted()).isEqualTo(3);

        // Test3  CREATE -> MATCH -> DELETE
        Movie movie = new Movie("Speed", 2019);
        Actor alice = new Actor("Alice Neeves");
        alice.actsIn(movie);
        session.save(movie);
        Movie m1 = session.load(Movie.class, movie.getId());
        session.delete(m1);
        session.delete(alice);
        Collection<Movie> ms = session.loadAll(Movie.class);
        assertThat(ms).hasSize(3);

        // Test4  DELETE -> LOADALL
        session.deleteAll(Actor.class);
        Collection<Actor> actors = session.loadAll(Actor.class);
        assertThat(actors).hasSize(0);

        // Test5  DELETE
        session.purgeDatabase();
        Collection<Movie> movies = session.loadAll(Movie.class);
        assertThat(movies).hasSize(0);
    }

    private static void testQuery() {
        log.info("----------------testQuery--------------------");
        // Test1  LOADALL
        Collection<Movie> movies = session.loadAll(Movie.class);
        assertThat(movies).hasSize(3);
        List<Movie> moviesList = new ArrayList<>(movies);
        for (int i = 0; i < moviesList.size(); i++) {
            log.info("Movie" + i + ": " + moviesList.get(i).getTitle()
                + ", released in " + moviesList.get(i).getReleased());
        }

        //Test2  LOADALL(Filter)
        Collection<Movie> moviesFilter = session.loadAll(Movie.class, new Filter("released", ComparisonOperator.LESS_THAN, 1995));
        assertThat(moviesFilter).hasSize(2);

        //Test3  MATCH
        HashMap<String, Object> parameters = new HashMap<>();
        parameters.put("title", "The Matrix");
        Iterable<Movie> actual = session
            .query(Movie.class, "MATCH (m:Movie{title: $title}) RETURN m", parameters);
        assertThat(actual.iterator().next().getTitle()).isEqualTo("The Matrix");

        // Test4
        HashMap<String, Object> parameters1 = new HashMap<>();
        parameters1.put("title", "The Matrix");
        Integer counts = session
            .queryForObject(Integer.class, "MATCH (n:Movie{title: $title})-[r]-(m:Actor) RETURN COUNT(m) AS counts", parameters1);
        assertThat(counts).isEqualTo(2);

        // Test5
        Result result6 = session.query("MATCH p = (m)-[rel]-(n) return p", Collections.emptyMap());
        assertThat(result6.queryResults()).hasSize(6);
    }

    private static void testCreate() {
        log.info("----------------testCreate--------------------");
        // Test1  CREATE -> LOAD
        session.query("CALL db.createVertexLabel('Movie', 'title', 'title', STRING, false, 'released', INT32, true)", emptyMap());
        session.query("CALL db.createVertexLabel('Actor', 'name', 'name', STRING, false)", emptyMap());
        session.query("CALL db.createEdgeLabel('ACTS_IN', '[]')", emptyMap());
        session.query("CALL db.createVertexLabel('Director', 'name', 'name', STRING, false, 'age', INT16, true)", emptyMap());
        session.query("CALL db.createEdgeLabel('DIRECT', '[]')", emptyMap());
        Movie movie1 = new Movie("Jokes", 1990);
        session.save(movie1);
        Movie m1 = session.load(Movie.class, movie1.getId());
        assertThat(movie1.getId()).isEqualTo(m1.getId());
        assertThat(movie1.getTitle()).isEqualTo(m1.getTitle());

        // Test2  CREATE -> LOAD
        Movie movie = new Movie("The Matrix", 1999);
        Actor keanu = new Actor("Keanu Reeves");
        keanu.actsIn(movie);
        Actor carrie = new Actor("Carrie-Ann Moss");
        carrie.actsIn(movie);
        session.save(movie);
        Movie matrix = session.load(Movie.class, movie.getId());
        for(Actor actor : matrix.getActors()) {
            log.info("Actor: " + actor.getName());
        }
        assertThat(matrix.getActors()).hasSize(2);

        // Test3  CREATE -> MATCH
        Result createResult = session.query("CREATE (n:Movie{title:\"The Shawshank Redemption\", released:1994})<-[r:DIRECT]-(n2:Director{name:\"Frank Darabont\", age:63})", emptyMap());
        QueryStatistics statistics = createResult.queryStatistics();
        assertThat(statistics.getNodesCreated()).isEqualTo(2);
        assertThat(statistics.getRelationshipsCreated()).isEqualTo(1);
        Result result = session.query("MATCH (m)-[r:DIRECT]->(n) return m,r,n", Collections.emptyMap());
        JSONObject r = (JSONObject) result.queryResults().iterator().next().get("r");
        for (String key : r.keySet()) {
            log.info(key + ": " + r.get(key));
        }
        assertThat(r).hasSize(7);
    }

    private static void testUpdate() {
        log.info("----------------testUpdate--------------------");
        // Test1: MATCH -> UPDATE -> LOAD
        HashMap<String, Object> parameters = new HashMap<>();
        parameters.put("name", "Keanu Reeves");
        Actor actor = session.queryForObject(Actor.class,
            "MATCH (actor:Actor{name:$name}) RETURN actor", parameters);
        actor.setName("NOBU Reeves");
        session.save(actor);
        Actor newactor = session.load(Actor.class, actor.getId());
        assertThat(newactor.getName()).isEqualTo("NOBU Reeves");
    }
}
