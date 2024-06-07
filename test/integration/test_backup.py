import logging
import pytest
import json

log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7072 --rpc_port 9092 --enable_plugin 1 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb",
             "cleanup_dir":["./testdb"]}

SERVEROPT_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7073 --rpc_port 9093 --enable_plugin 1 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb1",
               "cleanup_dir":["./testdb1"]}

CLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}

CLIENTOPT_1 = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}

IMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",
             "cleanup_dir":["./.import_tmp"]}

BACKUPOPT = {"cmd" : "./lgraph_backup --src ./testdb -dst ./testdb1",
             "cleanup_dir":[]}

BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared"],
            "so_name":["./scan_graph.so"]}

@pytest.fixture(scope="function")
def load_plugin(server, client):
    sort_so = BUILDOPT.get("so_name")[0]
    ret = client.loadProcedure(sort_so, "CPP", "sorter", "SO", "test plugin", True, "v1")
    assert ret[0]
    ret = client.listProcedures("CPP", "any")
    assert ret[0]
    plugins = json.loads(ret[1])
    assert len(plugins) == 1


class TestBackup:

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("backup_copy_dir", [BACKUPOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client_1", [CLIENTOPT_1], indirect=True)
    def test_backup_data(self, importor, server, backup_copy_dir, server_1, client_1):
        ret = client_1.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21


    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client_1", [CLIENTOPT_1], indirect=True)
    def test_unbackup_data(self, importor, server, server_1, client_1):
        ret = client_1.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0


    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    @pytest.mark.parametrize("backup_copy_dir", [BACKUPOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client_1", [CLIENTOPT_1], indirect=True)
    def test_backup_plugin(self, build_so, server, client, load_plugin, backup_copy_dir, server_1, client_1):
        ret = client_1.listProcedures("CPP", "any")
        assert ret[0]
        plugins = json.loads(ret[1])
        assert len(plugins[0]) == 1


    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client_1", [CLIENTOPT_1], indirect=True)
    def test_unbackup_plugin(self, build_so, server, client, load_plugin, server_1, client_1):
        ret = client_1.listProcedures("CPP", "any")
        assert ret[0]
        plugins = json.loads(ret[1])
        assert len(plugins) == 0