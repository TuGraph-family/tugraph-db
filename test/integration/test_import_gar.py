import pytest
import logging
from pathlib import Path

log = logging.getLogger(__name__)

class TestImportGar:
    config_path = Path.cwd().parent.parent / "test/resource/data/gar_test/ldbc_parquet/ldbc_sample.graph.yml"

    IMPORTOPT = {"cmd":f"./lgraph_import -c {config_path} --gar true --overwrite true --d gar_db",
                 "cleanup_dir":["./gar_db"]}

    SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./gar_db --port 27070 --rpc_port 27071 --log_dir '' ",
                 "cleanup_dir":["./gar_db"]}
    
    CLIENTOPT = {"host":"http://127.0.0.1:27071/LGraphHttpService/Query/", "user":"admin", "password":"73@TuGraph"}

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_import_gar(self, importor, server, rest_client):
        # test vertex label
        vertex_label_res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(vertex_label_res) == 1
        assert vertex_label_res[0]['label'] == 'person'

        # test edge label
        edge_label_res = rest_client.call_cypher("default", "CALL db.edgeLabels()")
        assert len(edge_label_res) == 1
        assert edge_label_res[0]['label'] == 'knows'

        # test vertex count
        vertex_count = rest_client.call_cypher("default", "MATCH (p:person) RETURN count(p)")
        assert vertex_count[0]['count(p)'] == 903

        # text edge count
        edge_count = rest_client.call_cypher("default", "MATCH ()-[r:knows]-() RETURN count(r)")
        assert edge_count[0]['count(r)'] == 6626 * 2

        # text vertex keys
        vertex_keys = rest_client.call_cypher("default", "MATCH (p:person) RETURN keys(p) LIMIT 1")
        assert "id" in vertex_keys[0]['keys(p)']
        assert "firstName" in vertex_keys[0]['keys(p)']
        assert "lastName" in vertex_keys[0]['keys(p)']
        assert "gender" in vertex_keys[0]['keys(p)']
        assert len(vertex_keys[0]['keys(p)']) == 30

        # test edge has 'creationDate' key
        edge_has_key = rest_client.call_cypher("default", "MATCH ()-[r]-() RETURN exists(r.creationDate) LIMIT 1")
        assert edge_has_key[0]['{EXISTS(r.creationDate)}'] == True
