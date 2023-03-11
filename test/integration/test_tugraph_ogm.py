import pytest
import logging

log = logging.getLogger(__name__)



class TestJavaOgm:

    BASHOPT = {
        "cmd" : "echo TestJavaOgm"
    }

    EXECOPT = {
        "cmd" : "java -jar tugraph-db-ogm-test-1.2.0.jar 127.0.0.1:29092 admin 73@TuGraph"
    }

    SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 27072 --rpc_port 29092",
                 "cleanup_dir":["./testdb"]}

    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECOPT], indirect=True)
    def test_java_ogm(self, bash, server, exec):
        pass
