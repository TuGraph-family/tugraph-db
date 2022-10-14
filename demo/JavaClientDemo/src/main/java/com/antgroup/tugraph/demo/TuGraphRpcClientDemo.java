package com.antgroup.tugraph.demo;
import java.io.IOException;
import com.antgroup.tugraph.TuGraphRpcClient;
import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TuGraphRpcClientDemo {
    static Logger log = LoggerFactory.getLogger(TuGraphRpcClientDemo.class);
    TuGraphRpcClient client = new TuGraphRpcClient("list://11.166.81.245:19090","admin", "73@TuGraph");
    void deleteAndCreate(String graphName) {
        try {
            // delete graph
            client.callCypher(String.format("CALL dbms.graph.deleteGraph('%s')", graphName), "default", 1000);
        } catch (Exception e) {
            log.info(e.toString());
        }
        // create graph
        client.callCypher(String.format("CALL dbms.graph.createGraph('%s', 'this is a demo graph', 20)", graphName), "default", 1000);
    }

    @Test
    // import data by configuration file
    void demo1() throws IOException {
        String graphName = "demo1";
        deleteAndCreate(graphName);
        // create vertex and edge labels described in 'schema' section of `movie/import_for_java_demo.json`
        client.importSchemaFromFile("movie/import_for_java_demo.json", graphName, 1000);

        // get all vertex labels
        String res = client.callCypher("CALL db.vertexLabels()", graphName, 1000);
        log.info("CALL db.vertexLabels() : " + res);
        // get all edge labels
        res = client.callCypher("CALL db.edgeLabels()", graphName, 1000);
        log.info("CALL db.edgeLabels() : " + res);

        // import vertex and edge data described in 'files' section of `movie/import_for_java_demo.json`
        client.importDataFromFile("movie/import_for_java_demo.json",",", true, 4, 0, graphName, 10000);

        // count all vertexs
        String vertexCount = client.callCypher("MATCH (n) RETURN count(n)", graphName, 1000);
        log.info(vertexCount);
        // count all edges
        String edgeCount = client.callCypher("MATCH (n)-[r]->(m) RETURN count(r)", graphName, 1000);
        log.info(edgeCount);
    }

    //
    @Test
    // import data by data block
    void demo2() throws IOException {
        String graphName = "demo2";
        deleteAndCreate(graphName);
        // create vertex and edge labels described in 'schema' section of `movie/import_for_java_demo.json`
        client.importSchemaFromFile("movie/import_for_java_demo.json", graphName, 1000);
/*
{
    "files": [
        {
            "format": "CSV",
            "label": "person",
            "columns": ["id", "name", "born", "poster_image"]
        }
    ]
}
*/
        String personDesc = "{\n" +
                "            \"files\": [\n" +
                "                {\n" +
                "                    \"format\": \"CSV\",\n" +
                "                    \"label\": \"person\",\n" +
                "                    \"columns\": [\"id\", \"name\", \"born\", \"poster_image\"]\n" +
                "                }\n" +
                "            ]\n" +
                "        }";

/*
2,Laurence Fishburne,1961,https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg
3,Carrie-Anne Moss,1967,https://image.tmdb.org/t/p/w185/8iATAc5z5XOKFFARLsvaawa8MTY.jpg
4,Hugo Weaving,1960,https://image.tmdb.org/t/p/w185/3DKJSeTucd7krnxXkwcir6PgT88.jpg
5,Gloria Foster,1933,https://image.tmdb.org/t/p/w185/ahwiARgfOYctk6sOLBBk5w7cfH5.jpg
6,Joe Pantoliano,1951,https://image.tmdb.org/t/p/w185/zBvDX2HWepvW9im6ikgoyOL2Xj0.jpg
7,Marcus Chong,1967,https://image.tmdb.org/t/p/w185/zYfXjMszFajTb93phn2Fi6LwEGN.jpg
8,Matt Doran,1976,https://image.tmdb.org/t/p/w185/gLpWm3azLiXgDPRWo23AnG5WM7O.jpg
9,Anthony Ray Parker,1958,https://image.tmdb.org/t/p/w185/iMHr0onfM8v4uVdPVnXxXx2xwwN.jpg
10,Keanu Reeves,1964,https://image.tmdb.org/t/p/w185/id1qIb7cZs2eQno90KsKwG8VLGN.jpg
*/

        String personData = "2,Laurence Fishburne,1961,https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg\n" +
                "3,Carrie-Anne Moss,1967,https://image.tmdb.org/t/p/w185/8iATAc5z5XOKFFARLsvaawa8MTY.jpg\n" +
                "4,Hugo Weaving,1960,https://image.tmdb.org/t/p/w185/3DKJSeTucd7krnxXkwcir6PgT88.jpg\n" +
                "5,Gloria Foster,1933,https://image.tmdb.org/t/p/w185/ahwiARgfOYctk6sOLBBk5w7cfH5.jpg\n" +
                "6,Joe Pantoliano,1951,https://image.tmdb.org/t/p/w185/zBvDX2HWepvW9im6ikgoyOL2Xj0.jpg\n" +
                "7,Marcus Chong,1967,https://image.tmdb.org/t/p/w185/zYfXjMszFajTb93phn2Fi6LwEGN.jpg\n" +
                "8,Matt Doran,1976,https://image.tmdb.org/t/p/w185/gLpWm3azLiXgDPRWo23AnG5WM7O.jpg\n" +
                "9,Anthony Ray Parker,1958,https://image.tmdb.org/t/p/w185/iMHr0onfM8v4uVdPVnXxXx2xwwN.jpg\n" +
                "10,Keanu Reeves,1964,https://image.tmdb.org/t/p/w185/id1qIb7cZs2eQno90KsKwG8VLGN.jpg";

        // import vertex data of 'person' label
        // personData : data block in csv format
        // personDesc : describe the detailed format of the data block
        client.importDataFromContent(personDesc, personData, ",", true, 4, graphName, 10000);

/*
{
    "files": [
        {
          "format": "CSV",
          "label": "movie",
          "columns": ["id","title","tagline","summary","poster_image","duration","rated"]
        }
    ]
}
*/
        String moiveDesc = "{\n" +
                "    \"files\": [\n" +
                "        {\n" +
                "          \"format\": \"CSV\",\n" +
                "          \"label\": \"movie\",\n" +
                "          \"columns\": [\"id\",\"title\",\"tagline\",\"summary\",\"poster_image\",\"duration\",\"rated\"]\n" +
                "        }\n" +
                "    ]\n" +
                "}";

/*
82,Pulp Fiction,Just because you are a character doesn't mean you have character.,placeholder text,http://image.tmdb.org/t/p/w185/dM2w364MScsjFf8pfMbaWUcWrR.jpg,154,R
130,Cloud Atlas,Everything is Connected,placeholder text,http://image.tmdb.org/t/p/w185/k9gWDjfXM80iXQLuMvPlZgSFJgR.jpg,172,R
457,The Shawshank Redemption,Fear can hold you prisoner. Hope can set you free.,placeholder text,http://image.tmdb.org/t/p/w185/9O7gLzmreU0nGkIB6K3BsJbzvNv.jpg,142,R
471,The Godfather,An offer you can't refuse.,placeholder text,http://image.tmdb.org/t/p/w185/d4KNaTrltq6bpkFS01pYtyXa09m.jpg,175,R
496,The Godfather: Part II,I don't feel I have to wipe everybody out\ Tom. Just my enemies.,placeholder text,http://image.tmdb.org/t/p/w185/tHbMIIF51rguMNSastqoQwR0sBs.jpg,200,R
517,The Good\ the Bad and the Ugly,For three men the Civil War wasn't hell. It was practice.,placeholder text,http://image.tmdb.org/t/p/w185/8PD1dgf0kQHtRawoSxp1jFemI1q.jpg,161,R
532,The Dark Knight,Why So Serious?,placeholder text,http://image.tmdb.org/t/p/w185/1hRoyzDtpgMU7Dz4JF22RANzQO7.jpg,152,PG-13
564,The Dark Knight Rises,The Legend Ends,placeholder text,http://image.tmdb.org/t/p/w185/dEYnvnUfXrqvqeRSqvIEtmzhoA8.jpg,165,PG-13
*/
        String moiveData = "82,Pulp Fiction,Just because you are a character doesn't mean you have character.,placeholder text,http://image.tmdb.org/t/p/w185/dM2w364MScsjFf8pfMbaWUcWrR.jpg,154,R\n" +
                "130,Cloud Atlas,Everything is Connected,placeholder text,http://image.tmdb.org/t/p/w185/k9gWDjfXM80iXQLuMvPlZgSFJgR.jpg,172,R\n" +
                "457,The Shawshank Redemption,Fear can hold you prisoner. Hope can set you free.,placeholder text,http://image.tmdb.org/t/p/w185/9O7gLzmreU0nGkIB6K3BsJbzvNv.jpg,142,R\n" +
                "471,The Godfather,An offer you can't refuse.,placeholder text,http://image.tmdb.org/t/p/w185/d4KNaTrltq6bpkFS01pYtyXa09m.jpg,175,R\n" +
                "496,The Godfather: Part II,I don't feel I have to wipe everybody out\\ Tom. Just my enemies.,placeholder text,http://image.tmdb.org/t/p/w185/tHbMIIF51rguMNSastqoQwR0sBs.jpg,200,R\n" +
                "517,The Good\\ the Bad and the Ugly,For three men the Civil War wasn't hell. It was practice.,placeholder text,http://image.tmdb.org/t/p/w185/8PD1dgf0kQHtRawoSxp1jFemI1q.jpg,161,R\n" +
                "532,The Dark Knight,Why So Serious?,placeholder text,http://image.tmdb.org/t/p/w185/1hRoyzDtpgMU7Dz4JF22RANzQO7.jpg,152,PG-13\n" +
                "564,The Dark Knight Rises,The Legend Ends,placeholder text,http://image.tmdb.org/t/p/w185/dEYnvnUfXrqvqeRSqvIEtmzhoA8.jpg,165,PG-13";

        // import vertex data of 'moive' label
        // moiveData : data block in csv format
        // moiveDesc : describe the detailed format of the data block
        client.importDataFromContent(moiveDesc, moiveData, ",", true, 4, graphName, 10000);



/*
{
    "files": [
        {
            "format":"CSV",
            "label":"acted_in",
            "SRC_ID":"person",
            "DST_ID":"movie",
            "columns": ["SRC_ID", "DST_ID", "role"]
        }
    ]
}
*/
        String actedInDesc = "{\n" +
                "    \"files\": [\n" +
                "        {\n" +
                "            \"format\":\"CSV\",\n" +
                "            \"label\":\"acted_in\",\n" +
                "            \"SRC_ID\":\"person\",\n" +
                "            \"DST_ID\":\"movie\",\n" +
                "            \"columns\": [\"SRC_ID\", \"DST_ID\", \"role\"]\n" +
                "        }\n" +
                "    ]\n" +
                "}";

/*
2,82,Morpheus
2,130,Morpheus
2,457,Morpheus
3,496,Trinity
3,517,Trinity
3,564,Trinity
*/

        String actedInData = "2,82,Morpheus\n" +
                "2,130,Morpheus\n" +
                "2,457,Morpheus\n" +
                "3,496,Trinity\n" +
                "3,517,Trinity\n" +
                "3,564,Trinity";

        // import edge data of 'acted_in' label
        // actedInData : data block in csv format
        // actedInDesc : describe the detailed format of the data block
        client.importDataFromContent(actedInDesc, actedInData, ",", true, 4, graphName, 10000);

        // get all vertex labels
        String res = client.callCypher("CALL db.vertexLabels()", graphName, 1000);
        log.info("CALL db.vertexLabels() : " + res);
        // get all edge labels
        res = client.callCypher("CALL db.edgeLabels()", graphName, 1000);
        log.info("CALL db.edgeLabels() : " + res);

        // count all vertexs
        String vertexCount = client.callCypher("MATCH (n) RETURN count(n)", graphName, 1000);
        log.info(vertexCount);
        // count all edges
        String edgeCount = client.callCypher("MATCH (n)-[r]->(m) RETURN count(r)", graphName, 1000);
        log.info(edgeCount);
    }

    @Test
    // create vertex and edge labels by cypher statements
    // create vertex and edge data by cypher statements
    void demo3() throws IOException {
        String graphName = "demo3";
        deleteAndCreate(graphName);
        // create vertex `person` label
        client.callCypher("CALL db.createVertexLabel(" +
                "'person'," +  // vertex name
                "'id'," +      // primary property
                "'id', int32, false," +
                "'name', string, false," +
                "'born', int32, true," +
                "'poster_image', string, true" +
                ")", graphName, 1000);

        // create vertex `movie` label
        client.callCypher("CALL db.createVertexLabel(" +
                "'movie'," +  // vertex name
                "'id'," +     // primary property
                "'id',int32, false," +
                "'title', string, false," +
                "'tagline', string, false," +
                "'summary', string, true," +
                "'poster_image', string, true," +
                "'duration', int32, false," +
                "'rated', string, true" +
                ")", graphName, 1000);

        // create edge `acted_in` label
        client.callCypher(("CALL db.createEdgeLabel(" +
                "'acted_in'," + // edge name
                "'[]'," +      // edge constraints. empty array means no constraints;
                "'role', string, false)"), graphName, 1000);

        // doc-zh/3.developer-document/2.cypher.md

        // 2,Laurence Fishburne,1961,https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg
        // create vertex by cypher
        client.callCypher("CREATE (n:person {" +
                "id: 2," +
                "name: 'Laurence Fishburne'," +
                "born: 1961," +
                "poster_image: 'https://image.tmdb.org/t/p/w185/mh0lZ1XsT84FayMNiT6Erh91mVu.jpg'})", graphName, 1000);

        // 130,Cloud Atlas,Everything is Connected,placeholder text,http://image.tmdb.org/t/p/w185/k9gWDjfXM80iXQLuMvPlZgSFJgR.jpg,172,R
        // create vertex by cypher
        client.callCypher("CREATE (n:movie {" +
                "id: 130," +
                "title: 'Cloud Atlas'," +
                "tagline:'Everything is Connected'," +
                "summary: 'placeholder text'," +
                "poster_image: 'http://image.tmdb.org/t/p/w185/k9gWDjfXM80iXQLuMvPlZgSFJgR.jpg'," +
                "duration: 172," +
                "rated: 'R'})", graphName, 1000);

        // create edge by cypher
        client.callCypher("MATCH (a:person), (b:movie) WHERE a.id = 2 AND b.id = 130 CREATE (a)-[r:acted_in {role: 'Morpheus'}]->(b)", graphName, 1000);

        // get all vertex labels
        String res = client.callCypher("CALL db.vertexLabels()", graphName, 1000);
        log.info("CALL db.vertexLabels() : " + res);
        // get all edge labels
        res = client.callCypher("CALL db.edgeLabels()", graphName, 1000);
        log.info("CALL db.edgeLabels() : " + res);

        // count all vertexs
        String vertexCount = client.callCypher("MATCH (n) RETURN count(n)", graphName, 1000);
        log.info(vertexCount);
        // count all edges
        String edgeCount = client.callCypher("MATCH (n)-[r]->(m) RETURN count(r)", graphName, 1000);
        log.info(edgeCount);
    }
}
