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
        vertex_label_res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        log.info(vertex_label_res)
        assert len(vertex_label_res) == 1
        assert vertex_label_res[0]['label'] == 'person'

        edge_label_res = rest_client.call_cypher("default", "CALL db.edgeLabels()")
        log.info(edge_label_res)
        assert len(edge_label_res) == 1
        assert edge_label_res[0]['label'] == 'knows'

        vertex_res = rest_client.call_cypher("default", "MATCH (p:person) RETURN count(p)")
        log.info(vertex_res)
        assert vertex_res[0]['count(p)'] == 903