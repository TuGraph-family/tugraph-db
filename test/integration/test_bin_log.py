import logging
import pytest
import json


log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7072 --rpc_port 9092 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb",
             "cleanup_dir":["./testdb"]}

SERVEROPT_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7073 --rpc_port 9093 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb1",
               "cleanup_dir":["./testdb1"]}


IMPORTOPT = {"cmd":"./lgraph_import --online true -c ./data/yago/yago.conf -r http://127.0.0.1:7072 -u admin -p 73@TuGraph",
             "cleanup_dir":["./.import_tmp"]}

CLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}


COPYSNAPOPT = {"src" : "./testdb", "dst" : "./testdb1"}

BINLOGOPT = {"cmd" : "./lgraph_binlog -a restore --host 127.0.0.1 --port 9093 -u admin -p 73@TuGraph -f ./testdb/binlog/*",
             "cleanup_dir":[]}

BINLOGOPT_1 = {"cmd" : "./lgraph_binlog -a restore --db_dir ./testdb1 -u admin -p 73@TuGraph -f ./testdb/binlog/*",
               "cleanup_dir":[]}




class TestBinLog:


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_remote_backup_none_binlog(self, server, importor, copy_snapshot, server_1, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("backup_binlog", [BINLOGOPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_remote_backup_with_binlog(self, server, importor, copy_snapshot, server_1, backup_binlog, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_local_backup_none_binlog(self, server, importor, copy_snapshot, server_1, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("copy_snapshot", [COPYSNAPOPT], indirect=True)
    @pytest.mark.parametrize("backup_binlog", [BINLOGOPT_1], indirect=True)
    @pytest.mark.parametrize("server_1", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_local_backup_with_binlog(self, server, importor, copy_snapshot, backup_binlog, server_1, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21







