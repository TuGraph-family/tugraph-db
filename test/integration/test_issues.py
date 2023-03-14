import logging
import pytest
import json
import os

log = logging.getLogger(__name__)

SERVEROPT_55_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093",
                  "cleanup_dir":[]}

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093",
             "cleanup_dir":["./testdb"]}

CLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}


SCHEMA = '''
{
    "schema": [
        {
            "label" : "v",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING", "optional":true}
            ]
        },
        {
            "label" : "e",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./vertex.csv",
            "format" : "CSV",
            "label" : "v",
            "columns" : ["id","name"]
        },
        {
            "path" : "./edge.csv",
            "format" : "CSV",
            "label" : "e",
            "SRC_ID" : "v",
            "DST_ID" : "v",
            "columns" : ["SRC_ID","DST_ID","weight"]
        }
    ]
} 
                    '''

@pytest.fixture(scope="function")
def file_builder(request):
    delimiter = request.param.get("delimiter")
    config_file_name = './import.config'
    vertex_file_name = './vertex.csv'
    edge_file_name = './edge.csv'
    with open(config_file_name, 'w') as f:
        f.write(SCHEMA)
        vertexes = [['001', 'name001'],
                ['002', 'name002']]
        edges = [['001', '002', '0.2'],
             ['001', '001', '0.3'],
             ['002', '002', '0.1']]

        with open(vertex_file_name, 'wb') as f:
            for v in vertexes:
                for i in range(0, len(v)):
                    if isinstance(v[i], str):
                        f.write(v[i].encode())
                    else:
                        f.write(v[i])
                    if i == len(v) - 1:
                        if isinstance('\n', str):
                            f.write('\n'.encode())
                        else:
                            f.write('\n')
                    else:
                        if isinstance(delimiter, str):
                            f.write(delimiter.encode())
                        else:
                            f.write(delimiter)
        with open(edge_file_name, 'wb') as f:
            for e in edges:
                for i in range(0, len(e)):
                    if isinstance(e[i], str):
                        f.write(e[i].encode())
                    else:
                        f.write(e[i])
                    if i == len(e) - 1:
                        if isinstance('\n', str):
                            f.write('\n'.encode())
                        else:
                            f.write('\n')
                    else:
                        if isinstance(delimiter, str):
                            f.write(delimiter.encode())
                        else:
                            f.write(delimiter)

    yield
    if os.path.exists(config_file_name):
        os.remove(config_file_name)
    if os.path.exists(vertex_file_name):
        os.remove(vertex_file_name)
    if os.path.exists(edge_file_name):
        os.remove(edge_file_name)


@pytest.fixture(scope="function")
def file_build_1():
    config_file_name = './import.config'
    vertex_file_name = './vertex.csv'
    edge_file_name = './edge.csv'
    with open(config_file_name, 'w') as f:
        f.write(SCHEMA)
    with open(vertex_file_name, 'w') as f:
        f.write('''
            001, name001
            002, name002
            ''')
    with open(edge_file_name, 'w') as f:
        f.write('''
            001, 002, 1.0
            ,002
            003, 004, 1.0
            name, other, 2.0
            , 001, 1.0
            ''')
    yield
    if os.path.exists(config_file_name):
        os.remove(config_file_name)
    if os.path.exists(vertex_file_name):
        os.remove(vertex_file_name)
    if os.path.exists(edge_file_name):
        os.remove(edge_file_name)

class TestIssues:


    # @pytest.mark.parametrize("server", [SERVEROPT_55_1], indirect=True)
    # @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    # def test_issues_55_1(self, server, client):
    #     ret = client.callCypher("CALL dbms.config.update({enable_audit_log:true})")
    #     assert ret[0]
    #     ret = client.callCypher("CALL dbms.config.list()", "default")
    #     configs = json.loads(ret[1])
    #     dict = {}
    #     for config in configs:
    #         dict[config["name"]] = config["value"]
    #     assert (dict["enable_audit_log"] == True)
    #
    #
    # @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    # @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    # def test_issues_55_2(self, server, client):
    #     ret = client.callCypher("CALL dbms.config.list()", "default")
    #     configs = json.loads(ret[1])
    #     dict = {}
    #     for config in configs:
    #         dict[config["name"]] = config["value"]
    #     assert (dict["enable_audit_log"] == True)


    FILEBUILDOPT = {"delimiter": ','}
    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --delimiter ','",
                "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("file_builder", [FILEBUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_1(self, file_builder, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 3)


    FILEBUILDOPT = {"delimiter": ')'}
    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --delimiter ')'",
                    "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("file_builder", [FILEBUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_2(self, file_builder, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 3)


    FILEBUILDOPT = {"delimiter": 'kkk'}
    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --delimiter 'kkk'",
                  "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("file_builder", [FILEBUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_3(self, file_builder, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 3)


    FILEBUILDOPT = {"delimiter": '!@'}
    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --delimiter '!@'",
                  "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("file_builder", [FILEBUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_4(self, file_builder, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 3)


    FILEBUILDOPT = {"delimiter": '\001'}
    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --delimiter '\\001'",
                  "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("file_builder", [FILEBUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_5(self, file_builder, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 3)


    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --continue_on_error true",
                  "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_59_6(self, file_build_1, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN COUNT(n)", "default")
        assert ret[0]
        has_vertex = json.loads(ret[1])[0]
        assert(has_vertex["COUNT(n)"] == 2)
        ret = client.callCypher("match (n)-[r]->(m) return COUNT(r)", "default")
        assert ret[0]
        has_edge = json.loads(ret[1])[0]
        assert(has_edge["COUNT(r)"] == 1)

    IMPORT_OPT = {"cmd":"./lgraph_import --config_file ./import.config --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1 --continue_on_error true",
                  "cleanup_dir":["./.import_tmp", "./testdb"]}

    @pytest.mark.parametrize("importor", [IMPORT_OPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issues_199(self, file_build_1, importor, server, client):
        ret = client.callCypher(
            "MATCH (n {id:1}), (m {id:1}), (o {id:2}) "
            "WHERE custom.myadd('asd')='1' RETURN 1")
        assert not ret[0]
        assert 'does not exist.' in ret[1]
