import liblgraph_client_python
import os
import json
import time
import shutil

log = ""
DEFAULT_ADMIN_NAME = "admin"
DEFAULT_ADMIN_PASS = "73@TuGraph"
IMPORT_SCHEMA = '''{"schema" :
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
IMPORT_DATA_PERSON_DESC = '''{"files": [
                                {
                                    "columns": ["name", "birthyear", "phone"],
                                    "format": "CSV",
                                    "header": 0,
                                    "label": "Person"
                                }]
                             }
                          '''
IMPORT_DATA_PERSON = '''
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

def importer(host, import_ort):
    client = start_ha_client(host, "29092")
    ret = client.callCypher("CALL dbms.ha.clusterInfo()", "default", timeout=10)
    master_info = ""
    confs = json.loads(ret[1])[0]["cluster_info"]
    for conf in confs:
        if conf.get("state") == "MASTER":
            master_info = conf.get("rest_address")
    os.system(import_ort["cmd"].format(master_info))
    for d in import_ort["cleanup_dir"]:
        if os.path.exists(d):
            shutil.rmtree(d)
    client.logout()

def start_ha_server(host, db):
    os.system(f"mkdir ha1 && cp -r ../../src/server/lgraph_ha.json "
              f"./lgraph_server ./resource ha1 "
              f"&& cd ha1 && ./lgraph_server --host {host} --port 27072 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29092 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(3)
    os.system(f"mkdir ha2 && cp -r ../../src/server/lgraph_ha.json "
              f"./lgraph_server ./resource ha2 "
              f"&& cd ha2 && ./lgraph_server --host {host} --port 27073 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29093 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(3)
    os.system(f"mkdir ha3 && cp -r ../../src/server/lgraph_ha.json "
              f"./lgraph_server ./resource ha3 "
              f"&& cd ha3 && ./lgraph_server --host {host} --port 27074 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29094 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(10)

def restart_ha_server(host, db):
    os.system(f"cd ha1 && ./lgraph_server --host {host} --port 27072 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29092 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(3)
    os.system(f"cd ha2 && ./lgraph_server --host {host} --port 27073 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29093 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(3)
    os.system(f"cd ha3 && ./lgraph_server --host {host} --port 27074 --enable_rpc "
              f"true --enable_ha true --ha_node_offline_ms 5000 --ha_node_remove_ms 10000 "
              f"--rpc_port 29094 --directory {db} --log_dir "
              f"./log  --ha_conf {host}:29092,{host}:29093,{host}:29094 --enable_plugin 1"
              f" -c lgraph_ha.json -d start")
    time.sleep(10)

def start_ha_client(host, port, user=DEFAULT_ADMIN_NAME, pwd=DEFAULT_ADMIN_PASS, urls=None):
    log.info("----------------start_ha_client--------------------")
    if urls:
        return liblgraph_client_python.client(urls, user, pwd)
    host_port = host + ":" + port
    url = host_port
    return liblgraph_client_python.client(url, user, pwd)

def build_procedure(procedure_name, procedure_path):
    log.info("----------------test_build_procedure--------------------")
    include_dir = "../../include"
    deps_include_dir = "../../tugraph-db/deps/install/include"
    lib_graph = "./liblgraph.so"
    cmd = f"g++ -fno-gnu-unique -fPIC -g --std=c++17 -I {include_dir} -I {deps_include_dir} " \
          f"-rdynamic -O3 -fopenmp -DNDEBUG -o {procedure_name} {procedure_path} {lib_graph} -shared"
    os.system(cmd)

def execute_cypher_and_assert(client, cypher, count):
    def execute_cypher():
        json_object = json.loads(result)[0]
        log.info("execute_cypher_and_assert" + result)
        assert "n" in json_object.keys()
        assert "identity" in json_object["n"].keys()
        assert json_object["n"]["identity"] == count
    ret, result = client.callCypher(cypher, "default", timeout=10)
    execute_cypher()
    servers = get_all_rpc_urls(client)
    for server in servers:
        ret, result = client.callCypher(cypher, "default", timeout=10,
                                        url=server)
        execute_cypher()

def get_all_rest_ports(client):
    res = []
    ret, result = client.callCypher("CALL dbms.ha.clusterInfo()", "default", timeout=10)
    json_object = json.loads(result)[0]
    log.info("get_all_rest_ports" + result)
    for o in json_object["cluster_info"]:
        res.append(o["rest_address"].split(":")[1])
    return res

def get_all_rpc_urls(client):
    res = []
    ret, result = client.callCypher("CALL dbms.ha.clusterInfo()", "default", timeout=10)
    json_object = json.loads(result)[0]
    log.info("get_all_rest_ports" + result)
    for o in json_object["cluster_info"]:
        res.append(o["rpc_address"])
    return res
