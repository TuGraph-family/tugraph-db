package com.antgroup.tugraph.test;
import java.io.IOException;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.JSONArray;
import java.io.UnsupportedEncodingException;

import com.antgroup.tugraph.TuGraphRpcException;
import lombok.extern.slf4j.Slf4j;
import com.antgroup.tugraph.TuGraphRpcClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TuGraphRpcClientTest {
    static Logger log = LoggerFactory.getLogger(TuGraphRpcClientTest.class);


    public static void loadPlugin(TuGraphRpcClient client) {
    log.info("----------------testLoadPlugin--------------------");
        try {
            client.callCypher("CALL db.dropDB()", "default", 10);
            boolean result = client.loadPlugin("./sortstr.so", "CPP", "sortstr", "SO", "test sortstr", true, "default", 1000);
            log.info("loadPlugin : " + result);
            assert(result);
            // should throw TuGraphRpcException
            result = client.loadPlugin("./scan_graph.so", "CPP", "scan_graph", "SO", "test scan_graph", true, "default", 1000);
            log.info("loadPlugin : " + result);
        } catch (IOException e) {
            log.info("catch IOException : " + e.getMessage());
        } catch (TuGraphRpcException e) {
            log.info("catch TuGraphRpcException : " + e.getMessage());
        }
    }

    public static void callPlugin(TuGraphRpcClient client) {
        log.info("----------------testCallPlugin--------------------");
        String result = client.callPlugin("CPP", "sortstr", "gecfb", 1000, false, "default", 1000);
        log.info("testCallPlugin : " + result);
        JSONObject jsonObject = JSONObject.parseObject(result);
        assert(jsonObject.containsKey("result"));
        assert("bcefg".equals(jsonObject.getString("result")));
    }

    public static void importSchemaFromContent(TuGraphRpcClient client) {
        log.info("----------------testImportSchemaFromContent--------------------");
        client.callCypher("CALL db.dropDB()", "default", 10);
        String schema = "{\"schema\" :" +
                        "    [" +
                        "         {" +
                        "             \"label\" : \"Person\"," +
                        "             \"type\" : \"VERTEX\"," +
                        "             \"primary\" : \"name\"," +
                        "             \"properties\" : [" +
                        "                 {\"name\" : \"name\", \"type\":\"STRING\"}," +
                        "                 {\"name\" : \"birthyear\", \"type\":\"INT16\", \"optional\":true}," +
                        "                 {\"name\" : \"phone\", \"type\":\"INT16\",\"unique\":true, \"index\":true}" +
                        "             ]" +
                        "         }," +
                        "        {" +
                        "            \"label\" : \"Film\"," +
                        "            \"type\" : \"VERTEX\"," +
                        "            \"primary\" : \"title\"," +
                        "            \"properties\" : [" +
                        "                {\"name\" : \"title\", \"type\":\"STRING\"} " +
                        "            ]" +
                        "        }," +
                        "       {" +
                        "	        \"label\": \"PLAY_IN\"," +
                        "	        \"type\": \"EDGE\"," +
                        "	        \"properties\": [{" +
                        "		        \"name\": \"role\"," +
                        "		        \"type\": \"STRING\", " +
                        "		        \"optional\": true " +
                        "	        }]," +
                        "	        \"constraints\": [" +
                        "		        [\"Person\", \"Film\"]" +
                        "	        ]" +
                        "       }" +
                        "    ]" +
                        "}";

        try {
            boolean ret = client.importSchemaFromContent(schema, "default", 1000);
            log.info("importSchemaFromContent : " + ret);
            assert(ret);
        } catch (UnsupportedEncodingException e) {
            log.info("catch exception : " + e.getMessage());
        }

        String res = client.callCypher("CALL db.vertexLabels()", "default", 10);
        log.info("db.vertexLabels() : " + res);
        JSONArray jsonArray = JSONArray.parseArray(res);
        assert(jsonArray.size() == 2);
        for (int idx = 0; idx < jsonArray.size(); ++idx) {
            JSONObject obj = (JSONObject)jsonArray.get(idx);
            assert("Person".equals(obj.getString("label")) || "Film".equals(obj.getString("label")));
        }
        res = client.callCypher("CALL db.edgeLabels()", "default", 10);
        log.info("db.edgeLabels() : " + res);
        JSONObject jsonObject = JSONObject.parseObject(res);
        assert(jsonObject.containsKey("edgeLabels"));
        assert("PLAY_IN".equals(jsonObject.getString("edgeLabels")));
    }

    public static void importDataFromContent(TuGraphRpcClient client) {
        log.info("----------------testImportDataFromContent--------------------");
        String personDesc = "{\"files\": [" +
                             "    {" +
                             "        \"columns\": [" +
                             "            \"name\"," +
                             "            \"birthyear\"," +
                             "            \"phone\"]," +
                             "        \"format\": \"CSV\"," +
                             "        \"header\": 0,"+
                             "        \"label\": \"Person\" " +
                             "        }"+
                             "    ]" +
                             "}";

        String person = "Rachel Kempson,1910,10086\n" +
                         "Michael Redgrave,1908,10087\n" +
                         "Vanessa Redgrave,1937,10088\n" +
                         "Corin Redgrave,1939,10089\n" +
                         "Liam Neeson,1952,10090\n" +
                         "Natasha Richardson,1963,10091\n" +
                         "Richard Harris,1930,10092\n" +
                         "Dennis Quaid,1954,10093\n" +
                         "Lindsay Lohan,1986,10094\n" +
                         "Jemma Redgrave,1965,10095\n" +
                         "Roy Redgrave,1873,10096\n" +
                         "John Williams,1932,10097\n" +
                         "Christopher Nolan,1970,10098\n";


        try {
            boolean ret = client.importDataFromContent(personDesc, person, ",", true, 16, "default", 1000);
            log.info("importDataFromContent : " + ret);
            assert(ret);
        } catch (UnsupportedEncodingException e) {
           log.info("catch exception : " + e.getMessage());
        }
        String res = client.callCypher("MATCH (n) RETURN COUNT(n)", "default", 10);
        log.info("MATCH (n) RETURN COUNT(n) : " + res);
        JSONObject jsonObject = JSONObject.parseObject(res);
        assert(jsonObject.containsKey("COUNT(n)"));
        assert(jsonObject.getIntValue("COUNT(n)") == 13);
    }

    public static void importSchemaFromFile(TuGraphRpcClient client) {
        log.info("----------------testImportSchemaFromFile--------------------");
        client.callCypher("CALL db.dropDB()", "default", 10);
        try {
            boolean ret = client.importSchemaFromFile("./data/yago/yago.conf", "default", 1000);
            log.info("importSchemaFromFile : " + ret);
            assert(ret);
        } catch (IOException e) {
            log.info("catch exception : " + e.getMessage());
        }

        String res = client.callCypher("CALL db.vertexLabels()", "default", 10);
        log.info("db.vertexLabels() : " + res);
        JSONArray array = JSONArray.parseArray(res);
        assert(array.size() == 3);
        for (int idx = 0; idx < array.size(); ++idx) {
            JSONObject obj = (JSONObject)array.get(idx);
            assert("Person".equals(obj.getString("label")) || "Film".equals(obj.getString("label"))
                ||  "City".equals(obj.getString("label")));
        }

        res = client.callCypher("CALL db.edgeLabels()", "default", 10);
        log.info("db.edgeLabels() : " + res);
        array = JSONArray.parseArray(res);
        assert(array.size() == 6);
        for (int idx = 0; idx < array.size(); ++idx) {
            JSONObject obj = (JSONObject)array.get(idx);
            assert("HAS_CHILD".equals(obj.getString("edgeLabels")) || "MARRIED".equals(obj.getString("edgeLabels"))
                || "BORN_IN".equals(obj.getString("edgeLabels")) || "DIRECTED".equals(obj.getString("edgeLabels"))
                || "WROTE_MUSIC_FOR".equals(obj.getString("edgeLabels"))
                || "ACTED_IN".equals(obj.getString("edgeLabels")));
        }

    }

    public static void importDataFromFile(TuGraphRpcClient client) {
        log.info("----------------testImportDataFromFile--------------------");
        try {
            boolean ret = client.importDataFromFile("./data/yago/yago.conf", ",", true, 16, 0, "default", 1000);
            log.info("importDataFromFile : " + ret);
            assert(ret);
        } catch (IOException e) {
            log.info("catch exception : " + e.getMessage());
        }
        String res = client.callCypher("MATCH (n:Person) RETURN COUNT(n)", "default", 1000);
        log.info("MATCH (n) RETURN COUNT(n) : " + res);
        JSONObject jsonObject = JSONObject.parseObject(res);
        assert(jsonObject.containsKey("COUNT(n)"));
        assert(jsonObject.getIntValue("COUNT(n)") == 13);

        res = client.callCypher("match(n) -[r]->(m) return count(r)", "default", 1000);
        log.info("match(n) -[r]->(m) return count(r) : " + res);
        jsonObject = JSONObject.parseObject(res);
        assert(jsonObject.containsKey("count(r)"));
        assert(jsonObject.getIntValue("count(r)") == 28);
    }

    public static TuGraphRpcClient startClient(String[] args) {
        log.info("----------------startClient--------------------");
        String hostPort = args[0];
        String user = args[1];
        String password = args[2];
        String url = "list://" + hostPort;
        TuGraphRpcClient client = new TuGraphRpcClient(url, user, password);
        return client;
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            log.info("java -jar target/tugraph-rpc-client-demo-3.1.0-jar-with-dependencies.jar [host:port] [user] [password]");
            return;
        }
        TuGraphRpcClient client = startClient(args);
        try {
            loadPlugin(client);
            callPlugin(client);
            importSchemaFromContent(client);
            importDataFromContent(client);
            importSchemaFromFile(client);
            importDataFromFile(client);
        } catch (TuGraphRpcException e) {
            log.info("Exception at "+e.GetErrorMethod()+" with errorCodeName: "+e.GetErrorCodeName()+" and error: "+e.GetError());
        }
    }
}
