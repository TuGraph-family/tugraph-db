import time
import logging
import ha_client_python_util
from ha_client_python_util import *

log = logging.getLogger(__name__)
ha_client_python_util.log = log

def get_node_edge_and_test(client):
    ret, result = client.callCypher("MATCH (n:Person) RETURN COUNT(n)", "default", timeout=1000)
    log.info("MATCH (n) RETURN COUNT(n) : " + result)
    json_object = json.loads(result)[0]
    assert "COUNT(n)" in json_object.keys()
    assert json_object["COUNT(n)"] == 13
    ret, result = client.callCypher("match(n)-[r]->(m) return count(r)", "default", timeout=1000)
    log.info("match(n)-[r]->(m) return count(r) : " + result)
    json_object = json.loads(result)[0]
    assert "count(r)" in json_object.keys()
    assert json_object["count(r)"] == 28

class TestHAStart:

    def group_restart(self):
        for i in range(27072, 27075):
            os.system("kill -2 $(ps -ef | grep " + str(i) + " | grep -v grep | awk '{print $2}')")
            time.sleep(1)
        os.system(f"cd ha1 && ./lgraph_server --host {self.host} --port 27072 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29092 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"cd ha2 && ./lgraph_server --host {self.host} --port 27073 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29093 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"cd ha3 && ./lgraph_server --host {self.host} --port 27074 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29094 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)

    def setup_class(self):
        self.host = str(os.popen("hostname -I").read()[:-2])

    def teardown(self):
        for i in range(27072, 27075):
            os.system("kill -9 $(ps -ef | grep " + str(i) + " | grep -v grep | awk '{print $2}')")
        for i in range(1, 4):
            os.system(f"rm -rf ha{i}")

    def test_conf_start(self):
        os.system(f"mkdir ha1 && ./lgraph_import -c data/yago/yago.conf -d ha1/db --continue_on_error 1 --overwrite 1")
        os.system(f"mkdir ha2 && ./lgraph_import -c data/yago/yago.conf -d ha2/db --continue_on_error 1 --overwrite 1")
        os.system(f"mkdir ha3 && ./lgraph_import -c data/yago/yago.conf -d ha3/db --continue_on_error 1 --overwrite 1")
        os.system(f"cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha1 "
                  f"&& cd ha1 && ./lgraph_server --host {self.host} --port 27072 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29092 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha2 "
                  f"&& cd ha2 && ./lgraph_server --host {self.host} --port 27073 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29093 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha3 "
                  f"&& cd ha3 && ./lgraph_server --host {self.host} --port 27074 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29094 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 -c lgraph_ha.json -d start")
        time.sleep(3)
        client = start_ha_client(self.host, "29092")
        get_node_edge_and_test(client)
        client.logout()
        client = start_ha_client(self.host, "29092", urls=[f"{self.host}:29092",
                                                                f"{self.host}:29093",
                                                                f"{self.host}:29094"])
        get_node_edge_and_test(client)
        client.logout()
        self.group_restart()
        time.sleep(10)
        client = start_ha_client(self.host, "29092")
        log.info("----------------test_conf_restart--------------------")
        get_node_edge_and_test(client)
        client.logout()

    def test_bootstrap_start(self):
        os.system(f"mkdir ha1 && ./lgraph_import -c data/yago/yago.conf -d ha1/db --continue_on_error 1 --overwrite 1")
        os.system(f"cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha1 "
                  f"&& cd ha1 && ./lgraph_server --host {self.host} --port 27072 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29092 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 --ha_bootstrap_role 1 "
                  f"-c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"mkdir ha2 && cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha2 "
                  f"&& cd ha2 && ./lgraph_server --host {self.host} --port 27073 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29093 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 --ha_bootstrap_role 2 "
                  f"-c lgraph_ha.json -d start")
        time.sleep(3)
        os.system(f"mkdir ha3 && cp -r ../../src/server/lgraph_ha.json "
                  f"./lgraph_server ./resource ha3 "
                  f"&& cd ha3 && ./lgraph_server --host {self.host} --port 27074 --enable_rpc "
                  f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
                  f"--rpc_port 29094 --directory ./db --log_dir "
                  f"./log  --ha_conf {self.host}:29092,{self.host}:29093,{self.host}:29094 --ha_bootstrap_role 2 "
                  f"-c lgraph_ha.json -d start")
        time.sleep(10)
        client = start_ha_client(self.host, "29092")
        log.info("----------------test_bootstrap_start--------------------")
        get_node_edge_and_test(client)
        client.logout()

        self.group_restart()
        time.sleep(10)
        client = start_ha_client(self.host, "29092")
        log.info("----------------test_bootstrap_restart--------------------")
        get_node_edge_and_test(client)
        client.logout()
