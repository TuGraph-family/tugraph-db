import logging
import pytest
import json

log = logging.getLogger(__name__)

SERVEROPT_bthread = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093 --enable_fulltext_index true",
               "cleanup_dir":["./testdb"]}

SERVEROPT_pthread = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 "
                           "--rpc_port 9093 --enable_fulltext_index true --use_pthread true",
                     "cleanup_dir":["./testdb"]}

CLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}

class TestCypher:
    @pytest.mark.parametrize("server", [SERVEROPT_bthread], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_fulltext_on_bthread(self, server, client):
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)", "default")
        assert ret[0]
        # deleteLabel will call full-text index
        ret = client.callCypher("CALL db.deleteLabel('vertex', 'actor')", "default")
        # calling full-text index will fail on bthread
        assert not ret[0]

    @pytest.mark.parametrize("server", [SERVEROPT_pthread], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_fulltext_on_pthread(self, server, client):
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)", "default")
        assert ret[0]
        # deleteLabel will call full-text index
        ret = client.callCypher("CALL db.deleteLabel('vertex', 'actor')", "default")
        # calling full-text index will succeed on pthread
        assert ret[0]