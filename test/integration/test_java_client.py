import pytest
import logging

log = logging.getLogger(__name__)



class TestJavaClient():

    BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_plugins/scan_graph.cpp ./liblgraph.so -shared",
                       "g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_plugins/sortstr.cpp ./liblgraph.so -shared"],
                "so_name":["./scan_graph.so", "./sortstr.so"]}

    BASHOPT = {
        "cmd" : "sh rpc_client/java/JavaClientTest/local_build.sh"
    }

    EXECOPT = {
        "cmd" : "java -jar -ea rpc_client/java/JavaClientTest/target/tugraph-rpc-client-test-3.1.0-jar-with-dependencies.jar 127.0.0.1:9092 admin 73@TuGraph"
    }

    SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092",
                 "cleanup_dir":["./testdb"]}

    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECOPT], indirect=True)
    def test_cpp_client(self, bash, build_so, server, exec):
        pass