import logging
import ha_client_python_util
from ha_client_python_util import *

log = logging.getLogger(__name__)
ha_client_python_util.log = log

IMPORTOPT = {"cmd":"./lgraph_import --online true -c ./data/yago/yago.conf -r http://{} -u admin -p 73@TuGraph",
             "cleanup_dir":["./.import_tmp"]}

BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared",
                   "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_procedures/sortstr.cpp ./liblgraph.so -shared"],
            "so_name":["./scan_graph.so", "./sortstr.so"]}

def stop_server():
    for i in range(27072, 27075):
        os.system("kill -9 $(ps -ef | grep " + str(i) + " | grep -v grep | awk '{print $2}')")

def db_check(host):
    client = start_ha_client(host, "29092")
    ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
    assert ret[0]
    res = json.loads(ret[1])
    assert len(res) == 21
    client.logout()

def procedure_load(host):
    client = start_ha_client(host, "29092")
    sort_so = BUILDOPT.get("so_name")[1]
    ret = client.loadProcedure(sort_so, "CPP", "sorter", "SO", "test plugin", True)
    assert ret[0]
    scan_so = BUILDOPT.get("so_name")[0]
    ret = client.loadProcedure(scan_so, "CPP", "scan", "SO", "test plugin", True)
    assert ret[0]
    client.logout()

def procedure_check(host):
    client = start_ha_client(host, "29092")
    ret = client.listProcedures("CPP")
    assert ret[0]
    plugins = json.loads(ret[1])
    assert len(plugins) == 2

    ret = client.callCypher("CALL db.plugin.listUserPlugins()")
    assert ret[0]
    plugins = json.loads(ret[1])
    assert len(plugins) == 2

    ret = client.callProcedure("CPP", "sorter", "eaozy", 10, False)
    assert ret[0]
    result = json.loads(ret[1])[0].get("result")
    assert result == "aeoyz"

    d = {"times" : 1, "scan_edges" : True}
    js = json.dumps(d)
    ret = client.callProcedure("CPP", "scan", js, 10, False)
    assert ret[0]
    result = json.loads(json.loads(ret[1])[0].get("result"))
    assert result.get("num_edges") == 28 and result.get("num_vertices") == 21
    client.logout()

class TestHAImport:
    def setup_class(self):
        self.host = str(os.popen("hostname -I").read()).strip()
        l = self.host.find(' ')
        if l != -1:
            self.host = self.host[:l]

    def teardown(self):
        stop_server()
        for i in range(1, 4):
            os.system(f"rm -rf ha{i}")

    def test_import_export(self):
        start_ha_server(self.host, "./db")
        importer(self.host, IMPORTOPT)
        db_check(self.host)
        stop_server()
        os.system(f"./lgraph_export -d ha1/db -e ha1/export.dir -u admin -p 73@TuGraph")
        os.system(f"./lgraph_import -c ha1/export.dir/import.config -d ha1/db.export --continue_on_error 1 --overwrite 1")
        os.system(f"./lgraph_import -c ha1/export.dir/import.config -d ha2/db.export --continue_on_error 1 --overwrite 1")
        os.system(f"./lgraph_import -c ha1/export.dir/import.config -d ha3/db.export --continue_on_error 1 --overwrite 1")
        restart_ha_server(self.host, "./db.export")
        db_check(self.host)

    def test_backup(self):
        start_ha_server(self.host, "./db")
        importer(self.host, IMPORTOPT)
        for cmd in BUILDOPT["cmd"]:
            os.system(cmd)
        db_check(self.host)
        procedure_load(self.host)
        procedure_check(self.host)
        stop_server()
        os.system(f"./lgraph_backup -s ha1/db -d ha1/db.backup")
        os.system(f"./lgraph_backup -s ha1/db -d ha2/db.backup")
        os.system(f"./lgraph_backup -s ha1/db -d ha3/db.backup")
        restart_ha_server(self.host, "./db.backup")
        db_check(self.host)
        procedure_check(self.host)
