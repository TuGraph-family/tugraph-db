import logging
import pytest
import json

log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7072 --rpc_port 9092 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb",
            "cleanup_dir":[]}

SERVEROPT_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --reset_admin_password 1 --port 7072 --rpc_port 9092 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb",
            "cleanup_dir":[]}

SERVEROPT_2 = {"cmd":"./lgraph_server -c lgraph_standalone.json --port 7072 --rpc_port 9092 --enable_backup_log true --host 0.0.0.0 --verbose 1 --directory ./testdb",
            "cleanup_dir":["./testdb"]}

CLIENTOPT = {"host":"0.0.0.0:9092", "user":"admin", "password":"73@TuGraph"}

class TestResetPassword:


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_change_password(self, server, client):
        ret = client.callCypher("CALL dbms.security.changePassword('73@TuGraph', '1111')")
        assert ret[0]

    @pytest.mark.parametrize("server_reset", [SERVEROPT_1], indirect=True)
    def test_reset_password(self, server_reset):
        pass

    # @pytest.mark.parametrize("server_reset_password", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT_2], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_login(self, server, client):
        pass
