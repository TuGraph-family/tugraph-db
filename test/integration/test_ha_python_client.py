import pytest
import time
import logging
import ha_client_python_util
from ha_client_python_util import *

log = logging.getLogger(__name__)
ha_client_python_util.log = log

class TestHAPythonClient:

    def setup_class(self):
        self.host = str(os.popen("hostname -I").read()[:-2])
        start_ha_server(self.host, "./db")
        self.client = start_ha_client(self.host, "29092")
        print("---------client start success!--------")

    def teardown_class(self):
        # self.client.logout()
        for i in range(27072, 27075):
            os.system("kill -9 $(ps -ef | grep " + str(i) + " | grep -v grep | awk '{print $2}')")
        for i in range(1, 4):
            os.system(f"rm -rf ha{i}")

    @pytest.mark.run(order=1)
    def test_cypher_before_import(self):
        res = json.loads(self.client.callCypher("MATCH (n) RETURN count(n)", "default", timeout=10)[1])
        assert len(res) == 0

    @pytest.mark.run(order=2)
    def test_import_schema_from_content(self):
        def node_result_check(res):
            log.info("db.vertexLabels() : " + res)
            json_array = json.loads(res)
            assert len(json_array) == 2
            for o in json_array:
                assert "Person" == o["label"] or "Film" == o["label"]
        def edge_result_check(res):
            log.info("db.edgeLabels() : " + res)
            json_object = json.loads(res)[0]
            assert "edgeLabels" in json_object.keys()
            assert "PLAY_IN" == json_object["edgeLabels"]
        log.info("----------------testImportSchemaFromContent--------------------")
        self.client.callCypher("CALL db.dropDB()", "default", timeout=10)
        schema = '''{"schema" :
            [
                {
                    "label" : "Person",
                    "type" : "VERTEX",
                    "primary" : "name",
                    "properties" : [
                        {"name" : "name", "type":"STRING"},
                        {"name" : "birthyear", "type":"INT16", "optional":true},
                        {"name" : "phone", "type":"INT16","unique":true, "index":true}
                    ]
                },
                {
                    "label" : "Film",
                    "type" : "VERTEX",
                    "primary" : "title",
                    "properties" : [
                        {"name" : "title", "type":"STRING"}
                    ]
                },
                {
                    "label": "PLAY_IN",
                    "type": "EDGE",
                    "properties": [
                        {"name": "role", "type": "STRING", "optional": true}
                    ],
                    "constraints": [ ["Person", "Film"] ]
                }
            ]
        }
        '''
        ret, result = self.client.importSchemaFromContent(schema, "default", timeout=1000)
        log.info("importSchemaFromContent : " + result)
        time.sleep(5)
        ret, result = self.client.callCypher("CALL db.vertexLabels()", "default", timeout=10)
        node_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.callCypher("CALL db.vertexLabels()",
                                                 "default", timeout=10, url=server)
            node_result_check(result)
        ret, result = self.client.callCypher("CALL db.edgeLabels()", "default", timeout=10)
        edge_result_check(result)
        for server in servers:
            ret, result = self.client.callCypher("CALL db.edgeLabels()",
                                                 "default", timeout=10, url=server)
            edge_result_check(result)

    @pytest.mark.run(order=3)
    def test_import_data_from_content(self):
        def node_result_check(res):
            log.info("MATCH (n) RETURN COUNT(n) : " + res)
            json_object = json.loads(res)[0]
            assert "COUNT(n)" in json_object.keys()
            assert json_object["COUNT(n)"] == 13
        log.info("----------------test_import_data_from_content--------------------")
        person_desc = '''{"files": [
            {
                "columns": ["name", "birthyear", "phone"],
                "format": "CSV",
                "header": 0,
                "label": "Person"
            }]
        }
        '''
        person = '''
            Rachel Kempson,1910,10086
            Michael Redgrave,1908,10087
            Vanessa Redgrave,1937,10088
            Corin Redgrave,1939,10089
            Liam Neeson,1952,10090
            Natasha Richardson,1963,10091
            Richard Harris,1930,10092
            Dennis Quaid,1954,10093
            Lindsay Lohan,1986,10094
            Jemma Redgrave,1965,10095
            Roy Redgrave,1873,10096
            John Williams,1932,10097
            Christopher Nolan,1970,10098
        '''
        ret, result = self.client.importDataFromContent(person_desc, person, ",", True, 16, "default", timeout=1000)
        log.info("importDataFromContent : " + result)
        time.sleep(5)
        ret, result = self.client.callCypher("MATCH (n) RETURN COUNT(n)", "default", timeout=10)
        node_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.callCypher("MATCH (n) RETURN COUNT(n)",
                                                 "default", timeout=10, url=server)
            node_result_check(result)

    @pytest.mark.run(order=4)
    def test_load_procedure(self):
        def procedure_result_check(res):
            log.info("listProcedure : " + res)
            inner_res = json.loads(res)
            assert len(inner_res) == 2
        log.info("----------------test_load_procedure--------------------")
        self.client.callCypher("CALL db.dropDB()", "default", timeout=10)
        build_procedure("./sortstr.so", "../../test/test_procedures/sortstr.cpp")
        ret, result = self.client.loadProcedure("./sortstr.so", "CPP", "sortstr", "SO", "test sortstr",
                                              True, "v1", "default")
        log.info("loadProcedure : " + result)
        build_procedure("./scan_graph.so", "../../test/test_procedures/scan_graph.cpp")
        self.client.loadProcedure("./scan_graph.so", "CPP", "scan_graph", "SO", "test scan_graph",
                                True, "v1", "default")
        time.sleep(5)
        ret, result = self.client.listProcedures("CPP")
        procedure_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.listProcedures("CPP", url=server)
            procedure_result_check(result)

    @pytest.mark.run(order=5)
    def test_call_delete_procedure(self):
        def call_procedure_result_check(res):
            log.info("test_call_procedure : " + res)
            json_object = json.loads(res)[0]
            assert "result" in json_object.keys()
            assert "bcefg" == json_object["result"]
        def list_procedure_result_check(res):
            log.info("listProcedure : " + res)
            inner_res = json.loads(res)
            assert len(inner_res) == 1
        log.info("----------------test_call_procedure--------------------")
        ret, result = self.client.callProcedure("CPP", "sortstr", "gecfb", 1000, False,
                                                "default")
        call_procedure_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.callProcedure("CPP", "sortstr", "gecfb", 1000, False,
                                                    "default", url=server)
            call_procedure_result_check(result)
        ret, result = self.client.deleteProcedure("CPP", "sortstr", "default")
        assert ret
        time.sleep(5)
        ret, result = self.client.listProcedures("CPP")
        list_procedure_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.listProcedures("CPP", url=server)
            list_procedure_result_check(result)

    @pytest.mark.run(order=6)
    def test_import_schema_from_file(self):
        def node_result_check(res):
            log.info("db.vertexLabels() : " + res)
            array = json.loads(res)
            assert len(array) == 3
            for o in array:
                assert "Person" == o["label"] or "Film" == o["label"] or "City" == o["label"]
        def edge_result_check(res):
            log.info("db.edgeLabels() : " + res)
            array = json.loads(res)
            assert len(array) == 6
            for o in array:
                assert "HAS_CHILD" == o["edgeLabels"] or "MARRIED" == o["edgeLabels"] or "BORN_IN" == o["edgeLabels"] \
                       or "DIRECTED" == o["edgeLabels"] or "WROTE_MUSIC_FOR" == o["edgeLabels"] \
                       or "ACTED_IN" == o["edgeLabels"]
        log.info("----------------test_import_schema_from_file--------------------")
        self.client.callCypher("CALL db.dropDB()", "default", 10)
        ret, result = self.client.importSchemaFromFile("./data/yago/yago.conf", "default", timeout=1000)
        log.info("importSchemaFromFile : " + result)
        time.sleep(5)

        ret, result = self.client.callCypher("CALL db.vertexLabels()", "default", timeout=10)
        node_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.callCypher("CALL db.vertexLabels()",
                                                 "default", timeout=10, url=server)
            node_result_check(result)
        ret, result = self.client.callCypher("CALL db.edgeLabels()", "default", timeout=10)
        edge_result_check(result)
        for server in servers:
            ret, result = self.client.callCypher("CALL db.edgeLabels()",
                                                 "default", timeout=10, url=server)
            edge_result_check(result)

    @pytest.mark.run(order=7)
    def test_import_data_from_file(self):
        def node_result_check(res):
            log.info("MATCH (n) RETURN COUNT(n) : " + res)
            json_object = json.loads(res)[0]
            assert "COUNT(n)" in json_object.keys()
            assert json_object["COUNT(n)"] == 13
        def edge_result_check(res):
            log.info("match(n)-[r]->(m) return count(r) : " + res)
            json_object = json.loads(res)[0]
            assert "count(r)" in json_object.keys()
            assert json_object["count(r)"] == 28
        log.info("----------------test_import_data_from_file--------------------")
        ret, result = self.client.importDataFromFile("./data/yago/yago.conf", ",", True, 16, 0, "default", timeout=1000)
        log.info("importDataFromFile : " + result)
        time.sleep(5)
        ret, result = self.client.callCypher("MATCH (n:Person) RETURN COUNT(n)", "default", timeout=1000)
        node_result_check(result)
        servers = get_all_rpc_urls(self.client)
        for server in servers:
            ret, result = self.client.callCypher("MATCH (n:Person) RETURN COUNT(n)",
                                                 "default", timeout=1000, url=server)
            node_result_check(result)
        ret, result = self.client.callCypher("match(n)-[r]->(m) return count(r)", "default", timeout=1000)
        edge_result_check(result)
        for server in servers:
            ret, result = self.client.callCypher("match(n)-[r]->(m) return count(r)",
                                                 "default", timeout=1000, url=server)
            edge_result_check(result)

    @pytest.mark.run(order=8)
    def test_cypher_after_import(self):
        log.info(self.client.callCypher("CREATE (p:Person{name:\"Test1\",birthyear:1988,phone:10000})",
                                         "default", timeout=10))
        time.sleep(5)
        execute_cypher_and_assert(self.client, '''MATCH (n:Person) WHERE n.name="Test1" RETURN n''', 21)

    @pytest.mark.run(order=9)
    def test_follower_restart(self):
        log.info("-------------------------stopping follower-------------------------")
        self.client.logout()
        os.system("kill -2 $(ps -ef | grep 27073 | grep -v grep | awk '{print $2}')")
        time.sleep(13)
        self.client = start_ha_client(self.host, "29092")
        time.sleep(7)

        get_all_rest_ports(self.client)
        log.info("-------------------------stop follower successfully-------------------------")
        log.info(self.client.callCypher("CREATE (p:Person{name:\"Test2\",birthyear:1988,phone:20000})",
                                        "default", timeout=10))

        log.info("-------------------------starting follower-------------------------")
        self.client.logout()
        os.system(f"cd ha2 && ./lgraph_server --host {self.host} --port 27073 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29093 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(13)
        self.client = start_ha_client(self.host, "29092")
        time.sleep(7)

        get_all_rest_ports(self.client)
        log.info("-------------------------start follower successfully-------------------------")
        execute_cypher_and_assert(self.client, '''MATCH (n:Person) WHERE n.name="Test2" RETURN n''', 22)
        self.client.logout()

    @pytest.mark.run(order=10)
    def test_leader_restart(self):
        log.info("-------------------------stopping leader-------------------------")
        os.system("kill -2 $(ps -ef | grep 27072 | grep -v grep | awk '{print $2}')")
        time.sleep(13)
        self.client = start_ha_client(self.host, "29093")
        time.sleep(7)

        get_all_rest_ports(self.client)
        log.info("-------------------------stop leader successfully-------------------------")
        log.info(self.client.callCypher("CREATE (p:Person{name:\"Test3\",birthyear:1988,phone:30000})",
                                        "default", timeout=10))

        log.info("-------------------------starting leader-------------------------")
        self.client.logout()
        os.system(f"cd ha1 && ./lgraph_server --host {self.host} --port 27072 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29092 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(13)
        self.client = start_ha_client(self.host, "29093")
        time.sleep(7)

        get_all_rest_ports(self.client)
        log.info("-------------------------start leader successfully-------------------------")
        execute_cypher_and_assert(self.client, '''MATCH (n:Person) WHERE n.name="Test3" RETURN n''', 23)
        self.client.logout()
