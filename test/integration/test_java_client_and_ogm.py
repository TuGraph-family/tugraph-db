import pytest
import logging

log = logging.getLogger(__name__)

BASHOPT = {
    "cmd" : "echo run TestJavaClientAndOGM"
}

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092 --enable_plugin 1",
             "cleanup_dir":["./testdb"]}

class TestJavaClientAndOGM:
    BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared",
                       "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_procedures/sortstr.cpp ./liblgraph.so -shared"],
                "so_name":["./scan_graph.so", "./sortstr.so"]}

    EXECCLIENTOPT = {
        "cmd" : "ls | grep jar | grep rpc-client-test | egrep -v 'javadoc|sources' | xargs -I{} java -jar -ea {} 127.0.0.1:9092 admin 73@TuGraph"
    }
    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECCLIENTOPT], indirect=True)
    def test_java_client(self, bash, build_so, server, exec):
        pass

    EXECHACLIENTOPT = {
        "cmd" : "ls | grep jar | grep rpc-client-test | egrep -v 'javadoc|sources' | xargs -I{} java -jar -ea {}"
    }
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECHACLIENTOPT], indirect=True)
    def test_ha_java_client(self, bash, exec):
        pass

    EXECOGMOPT = {
        "cmd" : "ls | grep jar | grep ogm-test | egrep -v 'javadoc|sources' | xargs -I{} java -jar {} 127.0.0.1:9092 admin 73@TuGraph"
    }
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECOGMOPT], indirect=True)
    def test_java_ogm(self, bash, server, exec):
        pass

    EXECHAOGMOPT = {
        "cmd" : "ls | grep jar | grep ogm-test | egrep -v 'javadoc|sources' | xargs -I{} java -jar {}"
    }
    @pytest.mark.parametrize("bash", [BASHOPT], indirect=True)
    @pytest.mark.parametrize("exec", [EXECHAOGMOPT], indirect=True)
    def test_ha_java_ogm(self, bash, exec):
        pass
