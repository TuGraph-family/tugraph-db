import logging
import pytest
import json

log = logging.getLogger(__name__)


SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb1 --port 7073 --rpc_port 9093",
             "cleanup_dir":["./testdb1"]}

CLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}

IMPORT_YAGO_OPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",
             "cleanup_dir":["./.import_tmp", "./testdb"]}

IMPORT_DEF_OPT = {"cmd":"./lgraph_import -c ./export/default/import.config -d ./testdb1",
             "cleanup_dir":["./.import_tmp", "./testdb1"]}

IMPORT_JSON_OPT = {"cmd":"./lgraph_import -c ./export/json/import.config -d ./testdb1",
                  "cleanup_dir":["./.import_tmp", "./testdb1"]}

IMPORT_CSV_OPT = {"cmd":"./lgraph_import -c ./export/csv/import.config -d ./testdb1",
                   "cleanup_dir":["./.import_tmp", "./testdb1"]}

EXPORT_DEF_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/default -g default -u admin -p 73@TuGraph",
                  "cleanup_dir":["./export"]}

EXPORT_JSON_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/json -g default -u admin -p 73@TuGraph -f json",
                  "cleanup_dir":["./export"]}

EXPORT_CSV_OPT = {"cmd":"./lgraph_export -d ./testdb -e ./export/csv -g default -u admin -p 73@TuGraph -f csv",
                   "cleanup_dir":["./export"]}

class TestExport:

    @pytest.mark.parametrize("importor", [IMPORT_YAGO_OPT], indirect=True)
    @pytest.mark.parametrize("exportor", [EXPORT_DEF_OPT], indirect=True)
    @pytest.mark.parametrize("importor_1", [IMPORT_DEF_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_export_default(self, importor, exportor, importor_1, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        log.info("res : %s", res)
        assert len(res) == 21


    @pytest.mark.parametrize("importor", [IMPORT_YAGO_OPT], indirect=True)
    @pytest.mark.parametrize("exportor", [EXPORT_JSON_OPT], indirect=True)
    @pytest.mark.parametrize("importor_1", [IMPORT_JSON_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_export_json(self, importor, exportor, importor_1, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        log.info("res : %s", res)
        assert len(res) == 21


    @pytest.mark.parametrize("importor", [IMPORT_YAGO_OPT], indirect=True)
    @pytest.mark.parametrize("exportor", [EXPORT_CSV_OPT], indirect=True)
    @pytest.mark.parametrize("importor_1", [IMPORT_CSV_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_export_csv(self, importor, exportor, importor_1, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        log.info("res : %s", res)
        assert len(res) == 21

