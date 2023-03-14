import pytest
import logging

log = logging.getLogger(__name__)

CLIENT_VERSION = "1.2.1" # modify this if you upgrade the java client

BASHOPT = {
    "cmd" : "echo run TestJavaClientAndOGM"
}

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092",
             "cleanup_dir":["./testdb"]}

class TestJavaClientAndOGM:
    BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_plugins/scan_graph.cpp ./liblgraph.so -shared",
                       "g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_plugins/sortstr.cpp ./liblgraph.so -shared"],
                "so_name":["./scan_graph.so", "./sortstr.so"]}

    EXECCLIENTOPT = {
        "cmd" : "java -jar -ea tugraph-db-java-rpc-client-test-%s.jar 127.0.0.1:9092 admin 73@TuGraph" % CLIENT_VERSION
    }
    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECCLIENTOPT], indirect=True)
    def test_java_client(self, bash, build_so, server, exec):
        pass

    EXECOGMOPT = {
        "cmd" : "java -jar tugraph-db-ogm-test-%s.jar 127.0.0.1:9092 admin 73@TuGraph" % CLIENT_VERSION
    }
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECOGMOPT], indirect=True)
    def test_java_ogm(self, bash, server, exec):
        pass
