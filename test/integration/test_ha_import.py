import logging
import time
import ha_client_python_util
from ha_client_python_util import *

log = logging.getLogger(__name__)
ha_client_python_util.log = log
import_host = str(os.popen("hostname -I").read()[:-2])

IMPORTOPT = {"cmd":"./lgraph_import --online true -c ./data/yago/yago.conf -r http://{} -u admin -p 73@TuGraph",
             "cleanup_dir":["./.import_tmp"]}

class TestHAImport:
    def setup_class(self):
        self.host = str(os.popen("hostname -I").read()[:-2])
        start_ha_server(self.host, "./db")

    def teardown_class(self):
        for i in range(27072, 27075):
            os.system("kill -9 $(ps -ef | grep " + str(i) + " | grep -v grep | awk '{print $2}')")
        for i in range(1, 4):
            os.system(f"rm -rf ha{i}")

    def test_online_import(self):
        client = start_ha_client(self.host, "29092")
        importer(self.host, IMPORTOPT)
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21