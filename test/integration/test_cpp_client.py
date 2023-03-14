import pytest
import logging

log = logging.getLogger(__name__)



class TestCppClient:

    BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_plugins/scan_graph.cpp ./liblgraph.so -shared",
                       "g++ -fno-gnu-unique -fPIC -g --std=c++11 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_plugins/sortstr.cpp ./liblgraph.so -shared"],
                "so_name":["./scan_graph.so", "./sortstr.so"]}

    BASHOPT = {
        "cmd" : "sh ./rpc_client/cpp/CppClientTest/compile.sh"
    }

    EXECOPT = {
        "cmd" : "rpc_client/cpp/CppClientTest/build/clienttest"
    }

    SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092",
                 "cleanup_dir":["./testdb"]}

    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECOPT], indirect=True)
    def test_cpp_client(self, bash, build_so, server, exec):
        pass