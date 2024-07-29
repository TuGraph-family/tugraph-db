import logging
import pytest
import json
import liblgraph_client_python

log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092 --enable_plugin 1",
             "cleanup_dir":["./testdb"]}

SERVEROPT_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7072 --rpc_port 9092 --enable_plugin 1",
             "cleanup_dir":[]}

CLIENTOPT = {"host":"127.0.0.1:9092", "user":"admin", "password":"73@TuGraph"}

IMPORTOPT = {"cmd":"./lgraph_import --config_file ./data/yago/yago.conf --dir ./testdb --user admin --password 73@TuGraph --graph default --overwrite 1",
             "cleanup_dir":["./testdb", "./.import_tmp"]}

BUILDOPT = {"cmd":["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./scan_graph.so ../../test/test_procedures/scan_graph.cpp ./liblgraph.so -shared",
                       "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./sortstr.so ../../test/test_procedures/sortstr.cpp ./liblgraph.so -shared"],
                "so_name":["./scan_graph.so", "./sortstr.so"]}

BUILDV2OPT = {"cmd": ["g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./v2_pagerank.so ../../test/test_procedures/v2_pagerank.cpp ./liblgraph.so -shared",
                      "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I ../../include -I ../../deps/install/include -rdynamic -O3 -fopenmp -DNDEBUG -o ./v2_test_path.so ../../test/test_procedures/v2_test_path.cpp ./liblgraph.so -shared"],
              "so_name": ["v2_pagerank.so", "v2_test_path.so"]}

IMPORTCONTENT = {
    "schema" : '''{"schema" : [
                    {
                        "label" : "Person",
                        "type" : "VERTEX",
                        "primary" : "name",
                        "properties" : [
                            {"name" : "name", "type":"STRING"},
                            {"name" : "birthyear", "type":"INT16", "optional":true},
                            {"name" : "phone", "type":"INT16","unique":true, "index":true}
                        ]
                    },
                {
                    "label" : "City",
                    "type" : "VERTEX",
                    "primary" : "name",
                    "properties" : [
                        {"name" : "name", "type":"STRING"}
                    ]
                },
                {
                    "label" : "Film",
                    "type" : "VERTEX",
                    "primary" : "title",
                    "properties" : [
                        {"name" : "title", "type":"STRING"}
                    ]
                },
                {"label" : "HAS_CHILD", "type" : "EDGE"},
                {"label" : "MARRIED", "type" : "EDGE"},
                {
                    "label" : "BORN_IN",
                    "type" : "EDGE",
                    "properties" : [
                        {"name" : "weight", "type":"FLOAT", "optional":true}
                    ]
                },
                {"label" : "DIRECTED", "type" : "EDGE"},
                {"label" : "WROTE_MUSIC_FOR", "type" : "EDGE"},
                {
                    "label" : "ACTED_IN",
                    "type" : "EDGE",
                    "properties" : [
                        {"name" : "charactername", "type":"STRING"}
                    ]
                },
                {
                    "label": "PLAY_IN",
                    "type": "EDGE",
                    "properties": [{
                        "name": "role",
                        "type": "STRING",
                        "optional": true
                    }],
                    "constraints": [
                        ["Person", "Film"]
                    ]
                }
    ]}''',

    "person_desc" : '''{"files": [
    {
        "columns": [
            "name",
            "birthyear",
            "phone"
        ],
        "format": "CSV",
        "header": 0,
        "label": "Person"
        }
        ]
    }''',

    "person" : '''Rachel Kempson,1910,10086
Michael Redgrave,1908,10087
Vanessa Redgrave,1937,10088
Corin Redgrave,1939,10089
Liam Neeson,1952,10090
Natasha Richardson,1963,10091
Richard Harris,1930,10092
Dennis Quaid,1954,10093
Lindsay Lohan,1986,10094
Jemma Redgrave,1965,10095
Roy Redgrave,1873,10096
John Williams,1932,10097
Christopher Nolan,1970,10098''',

    "city_desc" : '''
    {
    "files": [
        {
            "columns": [
                "name"
            ],
            "format": "CSV",
            "header": 1,
            "label": "City"
        }
    ]
    }
    ''',

    "city" : '''Header Line
New York
London
Houston''',

    "film_desc" : '''
    {
    "files": [
        {
            "columns": [
                "title"
            ],
            "format": "CSV",
            "header": 0,
            "label": "Film"
        }
    ]
    }
    ''',

    "film" : '''"Goodbye, Mr. Chips"
Batman Begins
Harry Potter and the Sorcerer's Stone
The Parent Trap
Camelot''',

    "has_child_desc" : '''
    {
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "HAS_CHILD"
        }
    ]
    }
    ''',

    "has_child" : '''Rachel Kempson,Vanessa Redgrave
Rachel Kempson,Corin Redgrave
Michael Redgrave,Vanessa Redgrave
Michael Redgrave,Corin Redgrave
Corin Redgrave,Jemma Redgrave
Vanessa Redgrave,Natasha Richardson
Roy Redgrave,Michael Redgrave''',

    "married_desc" : '''
    {
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "MARRIED"
        }
    ]
    }
    ''',

    "married" : '''Rachel Kempson,Michael Redgrave
Michael Redgrave,Rachel Kempson
Natasha Richardson,Liam Neeson
Liam Neeson,Natasha Richardson''',

    "born_in_desc" : '''
    {
    "files": [
        {
            "DST_ID": "City",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "weight"
            ],
            "format": "CSV",
            "header": 0,
            "label": "BORN_IN"
        }
    ]
    }
    ''',

    "born_in" : '''Vanessa Redgrave,London,20.21
Natasha Richardson,London,20.18
Christopher Nolan,London,19.93
Dennis Quaid,Houston,19.11
Lindsay Lohan,New York,20.62
John Williams,New York,20.55''',

    "directed_desc" : '''
    {
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "DIRECTED"
        }
    ]
    }
    ''',

    "directed" : '''Christopher Nolan,Batman Begins''',

    "wrote_desc" : '''
    {
    "files": [
    {
        "DST_ID": "Film",
        "SRC_ID": "Person",
        "columns": [
            "SRC_ID",
            "DST_ID"
        ],
        "format": "CSV",
        "header": 0,
        "label": "WROTE_MUSIC_FOR"
    }
    ]
    }
    ''',

    "wrote" : '''John Williams,Harry Potter and the Sorcerer's Stone
John Williams,"Goodbye, Mr. Chips"''',

    "acted_in_desc" : '''
    {
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "charactername"
            ],
            "format": "CSV",
            "header": 0,
            "label": "ACTED_IN"
        }
    ]
    }
    ''',

    "acted_in" : '''Michael Redgrave,"Goodbye, Mr. Chips",The Headmaster
Vanessa Redgrave,Camelot,Guenevere
Richard Harris,Camelot,King Arthur
Richard Harris,Harry Potter and the Sorcerer's Stone,Albus Dumbledore
Natasha Richardson,The Parent Trap,Liz James
Dennis Quaid,The Parent Trap,Nick Parker
Lindsay Lohan,The Parent Trap,Halle/Annie
Liam Neeson,Batman Begins,Henri Ducard'''
}

class TestProcedure:

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_flushdb(self, importor, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21

        ret = client.callCypher("CALL db.flushDB()", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_subgraph(self, server, client):
        ret = client.callCypher("CALL db.subgraph([0,1,2])", "default")
        assert ret[0]

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_vertex_label(self, server, client):
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', 'string', false, 'age', 'int8', true)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', 'string', false, 'age', 'int8', true)", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.createVertexLabel('dirctor', 'name', 'name', 'string', false, 'age', 'int8', true)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.vertexLabels()", "default")
        assert ret[0]
        labels = json.loads(ret[1])
        for label in labels:
            assert label.get("label") == "actor" or label.get("label") == "dirctor"
        ret = client.callCypher("CALL db.deleteLabel('vertex', 'dirctor')", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.vertexLabels()", "default")
        label = json.loads(ret[1])[0]
        assert label.get("label") == "actor"


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_edge_label(self, server, client):
        ret = client.callCypher("CALL db.createEdgeLabel('followed', '[]', 'address', 'string', false, 'date', 'int32', false)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.createEdgeLabel('followed', '[]', 'address', 'string', false, 'date', 'int32', false)", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.createEdgeLabel('married', '[]', 'address', 'string', false, 'date', 'int32', false)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.edgeLabels()", "default")
        assert ret[0]
        labels = json.loads(ret[1])
        for label in labels:
            assert label.get("label") == "followed" or label.get("label") == "married"
        ret = client.callCypher("CALL db.deleteLabel('edge', 'followed')", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.edgeLabels()", "default")
        assert ret[0]
        label = json.loads(ret[1])[0]
        assert label.get("label") == "married"


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_index(self, server, client):
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', 'string', false, 'age', 'int8', true)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.createVertexLabel('actor', 'name', 'name', 'string', false, 'age', 'int8', true)", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.indexes()", "default")
        assert ret[0]
        index = json.loads(ret[1])[0]
        assert index.get("field") == "name" and index.get("label") == "actor"
        ret = client.callCypher("CALL db.addIndex('actor', 'age', false)", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.indexes()", "default")
        assert ret[0]
        index = json.loads(ret[1])
        assert len(index) == 2
        ret = client.callCypher("CALL db.deleteIndex('actor', 'name')", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.deleteIndex('actor', 'age')", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.indexes()", "default")
        assert ret[0]
        index = json.loads(ret[1])[0]
        assert index.get("field") == "name" and index.get("label") == "actor"


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_warmup(self, server, client):
        #TODO import data and get memory info
        ret = client.callCypher("CALL db.warmup()", "default")
        assert ret[0]
        #TODO import data and get memory info


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_create_label(self, server, client):
        ret = client.callCypher("CALL db.createLabel('vertex', 'animal', 'sleep', ['eat', 'string', true], ['sleep', 'int8', false])", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.createLabel('vertex', 'animal', 'sleep', ['eat', 'string', true], ['sleep', 'int8', false])", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.vertexLabels()", "default")
        assert ret[0]
        label = json.loads(ret[1])[0]
        assert label.get("label") == "animal"
        ret = client.callCypher("CALL db.getLabelSchema('vertex', 'animal')", "default")
        assert ret[0]
        fields = json.loads(ret[1])
        assert len(fields) == 2
        for field in fields:
            if field.get("name") == "sleep":
                field.get("optional") == True
            if field.get("name") == "eat":
                field.get("optional") == False


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_label_field(self, server, client):
        ret = client.callCypher("CALL db.createLabel('vertex', 'animal', 'sleep', ['eat', 'string', true], ['sleep', 'int8', false])", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.alterLabelAddFields('vertex', 'animal', ['run', 'string', '',true], ['jeep', 'int8', 10,false])", "default")
        assert ret[0]

        ret = client.callCypher("CALL db.getLabelSchema('vertex', 'animal')", "default")
        assert ret[0]
        fields = json.loads(ret[1])
        assert len(fields) == 4
        for field in fields:
            if field.get("name") == "run":
                field.get("type") == "string"
            if field.get("name") == "jeep":
                field.get("type") == "int8"

        ret = client.callCypher("CALL db.alterLabelModFields('vertex', 'animal',['run', 'int8', false], ['jeep', 'int32', true])", "default")
        assert ret[0]

        ret = client.callCypher("CALL db.getLabelSchema('vertex', 'animal')", "default")
        assert ret[0]
        fields = json.loads(ret[1])
        for field in fields:
            if field.get("name") == "run":
                field.get("type") == "int8"
            if field.get("name") == "jeep":
                field.get("type") == "int32"

        ret = client.callCypher("CALL db.alterLabelDelFields('vertex', 'animal', ['eat', 'sleep'])", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL db.alterLabelDelFields('vertex', 'animal', ['eat', 'run'])", "default")
        assert ret[0]
        ret = client.callCypher("CALL db.getLabelSchema('vertex', 'animal')", "default")
        assert ret[0]
        fields = json.loads(ret[1])
        assert len(fields) == 2
        for field in fields:
            assert field.get("name") == "jeep" or field.get("name") == "sleep"


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_procedure(self, server, client):
        ret = client.callCypher("CALL dbms.procedures()", "default")
        assert ret[0]
        procedures = json.loads(ret[1])
        #TODO when this assert failed , you should add the additional procedure test code or remove the deleted procedure test code
        log.info("procedures count : %s", len(procedures))
        assert len(procedures) == 104


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_graph(self, server, client):
        ret = client.callCypher("CALL dbms.graph.createGraph('test_graph1', 'this is a test graph', 20)", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.graph.createGraph('test_graph1', 'this is a test graph', 20)", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.graph.createGraph('test_graph2', 'this is a test graph', 100)", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.graph.listGraphs()", "default")
        assert ret[0]
        graphs = json.loads(ret[1])
        assert len(graphs) == 3
        for graph in graphs:
            assert graph.get("graph_name") == "test_graph1" or graph.get("graph_name") == "test_graph2" or graph.get("graph_name") == "default"

        ret = client.callCypher("CALL dbms.graph.modGraph('test_graph1', {max_size_GB:200, description:'modify graph1 desc'})", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.graph.listGraphs()", "default")
        assert ret[0]
        graphs = json.loads(ret[1])
        for graph in graphs:
            if graph.get("graph_name") == "test_graph1":
                assert graph.get("configuration").get("description") == "modify graph1 desc"
                assert graph.get("configuration").get("max_size_GB") == 200

        ret = client.callCypher("CALL dbms.graph.deleteGraph('test_graph1')", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.graph.deleteGraph('test_graph1')", "default")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.graph.listGraphs()", "default")
        assert ret[0]
        graphs = json.loads(ret[1])
        assert len(graphs) == 2


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_allow_list(self, server, client):
        ret = client.callCypher("CALL dbms.security.listAllowedHosts()", "default")
        assert ret[0]
        allows = json.loads(ret[1])
        assert len(allows) == 0
        ret = client.callCypher("CALL dbms.security.addAllowedHosts('172.172.1.1', '127.0.0.1')", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listAllowedHosts()", "default")
        assert ret[0]
        allows = json.loads(ret[1])
        assert len(allows) == 2
        for allow in allows:
            allow.get("host") == "172.172.1.1" or allow.get("host") == "127.0.0.1"

        ret = client.callCypher("CALL dbms.security.deleteAllowedHosts('172.172.1.1')", "default")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listAllowedHosts()", "default")
        assert ret[0]
        allows = json.loads(ret[1])
        assert len(allows) == 1


    @pytest.mark.parametrize("server", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_configration(self, server, client):
        ret = client.callCypher("CALL dbms.config.list()", "default")
        assert ret[0]
        confs = json.loads(ret[1])
        for conf in confs:
            if conf.get("name") == "durable":
                assert conf.get("value") == False
            if conf.get("name") == "enable_audit_log":
                assert conf.get("value") == False

        ret = client.callCypher("CALL dbms.config.update({durable:true,enable_audit_log:true})", "default")
        assert ret[0]

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_configration_two(self, server, client):
        ret = client.callCypher("CALL dbms.config.list()", "default")
        assert ret[0]
        confs = json.loads(ret[1])
        for conf in confs:
            if conf.get("name") == "durable":
                assert conf.get("value") == True
            if conf.get("name") == "enable_audit_log":
                assert conf.get("value") == True

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_system_info(self, server, client):
        ret = client.callCypher("CALL dbms.system.info()", "default")
        assert ret[0]
        confs = json.loads(ret[1])
        for conf in confs:
            key = conf.get("name")
            if key not in ["lgraph_version", "up_time", "git_branch", "git_commit", "web_commit", "cpp_id", "cpp_version", "python_version"]:
                assert False

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_rule(self, server, client):
        ret = client.callCypher("CALL dbms.security.createRole('test_role1', 'this is test role1')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.createRole('test_role1', 'this is test role1')")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.security.createRole('test_role2', 'this is test role2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listRoles()")
        assert ret[0]
        roles = json.loads(ret[1])
        assert len(roles) == 3
        for role in roles:
            assert role.get("role_name") == "admin" or role.get("role_name") == "test_role1" or role.get("role_name") == "test_role2"

        ret = client.callCypher("CALL dbms.security.modRoleDesc('test_role2', 'modified role2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.disableRole('test_role2', true)")
        assert ret[0]

        ret = client.callCypher("CALL dbms.security.getRoleInfo('test_role2')")
        assert ret[0]
        info = json.loads(ret[1])[0].get("role_info")
        assert info.get("description") == "modified role2"
        assert info.get("disabled") == True
        assert info.get("permissions") == None

        ret = client.callCypher("CALL dbms.graph.createGraph('test_graph1', 'this is a test graph', 20)")
        assert ret[0]
        ret = client.callCypher("CALL dbms.graph.createGraph('test_graph2', 'this is a test graph', 100)")
        assert ret[0]

        ret = client.callCypher("CALL dbms.security.rebuildRoleAccessLevel('test_role2', {test_graph1:'READ', test_graph2:'WRITE'})")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getRoleInfo('test_role2')")
        assert ret[0]
        permissions = json.loads(ret[1])[0].get("role_info").get("permissions")
        assert len(permissions) == 2
        assert permissions.get("test_graph1") == "READ" or permissions.get("test_graph1") == "WRITE"

        ret = client.callCypher("CALL dbms.security.modRoleAccessLevel('test_role2', {test_graph1:'FULL', test_graph2:'NONE'})")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getRoleInfo('test_role2')")
        assert ret[0]
        permissions = json.loads(ret[1])[0].get("role_info").get("permissions")
        assert len(permissions) == 1
        assert permissions.get("test_graph1") == "FULL"

        ret = client.callCypher("CALL dbms.security.deleteRole('test_role1')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.deleteRole('test_role1')")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.security.deleteRole('test_role2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listRoles()")
        assert ret[0]
        role = json.loads(ret[1])[0]
        assert role.get("role_name") == "admin"


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_user(self, server, client):
        ret = client.callCypher("CALL dbms.security.createUser('test_user1', 'this is password')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.createUser('test_user1', 'this is password')")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.security.createUser('test_user2', 'this is password')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.setCurrentDesc('modified user desc')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listUsers()")
        assert ret[0]
        users = json.loads(ret[1])
        assert len(users) == 3
        for user in users:
            assert user.get("user_name") == "admin" or user.get("user_name") == "test_user1" or user.get("user_name") == "test_user2"
        for user in users:
            if user.get("user_name") == "admin":
                assert user.get("user_info").get("description") == "modified user desc"

        ret = client.callCypher("CALL dbms.security.showCurrentUser()")
        assert ret[0]
        cur_user = json.loads(ret[1])[0]
        assert cur_user.get("current_user") == "admin"

        ret = client.callCypher("CALL dbms.security.setUserDesc('test_user2', 'modified user2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.disableUser('test_user2', true)")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getUserInfo('test_user2')")
        assert ret[0]
        info = json.loads(ret[1])[0].get("user_info")
        assert info.get("description") == "modified user2"
        assert info.get("disabled") == True

        ret = client.callCypher("CALL dbms.security.createRole('test_role1', 'this is test role1')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.createRole('test_role2', 'this is test role2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.createRole('test_role3', 'this is test role3')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.addUserRoles('test_user1', ['test_role1', 'test_role2'])")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getUserInfo('test_user1')")
        assert ret[0]
        roles = json.loads(ret[1])[0].get("user_info").get("roles")
        assert len(roles) == 3
        assert roles[0] == "test_role1" and roles[1] == "test_role2" and roles[2] == "test_user1"

        ret = client.callCypher("CALL dbms.security.rebuildUserRoles('test_user1', ['test_role3'])")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getUserInfo('test_user1')")
        assert ret[0]
        roles = json.loads(ret[1])[0].get("user_info").get("roles")
        assert len(roles) == 2
        assert roles[0] == "test_role3" and roles[1] == "test_user1"

        ret = client.callCypher("CALL dbms.security.deleteUserRoles('test_user1', ['test_role3'])")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.getUserInfo('test_user1')")
        assert ret[0]
        roles = json.loads(ret[1])[0].get("user_info").get("roles")
        assert len(roles) == 1
        assert roles[0] == "test_user1"

        ret = client.callCypher("CALL dbms.security.deleteUser('test_user1')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.deleteUser('test_user1')")
        assert ret[0] == False
        ret = client.callCypher("CALL dbms.security.deleteUser('test_user2')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.listUsers()")
        assert ret[0]
        user = json.loads(ret[1])[0]
        assert user.get("user_name") == "admin"

    @pytest.mark.parametrize("build_so", [BUILDOPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_plugin(self, build_so, importor, server, client):
        sort_so = BUILDOPT.get("so_name")[1]
        ret = client.loadProcedure(sort_so, "CPP", "sorter", "SO", "test plugin", True, "v1")
        assert ret[0]
        scan_so = BUILDOPT.get("so_name")[0]
        ret = client.loadProcedure(scan_so, "CPP", "scan", "SO", "test plugin", True, "v1")
        assert ret[0]
        ret = client.listProcedures("CPP", "any")
        assert ret[0]
        plugins = json.loads(ret[1])
        assert len(plugins) == 2

        ret = client.callCypher("CALL db.plugin.listUserPlugins()")
        assert ret[0]
        plugins = json.loads(ret[1])
        assert len(plugins) == 2

        ret = client.callProcedure("CPP", "sorter", "eaozy", 10, False)
        assert ret[0]
        result = json.loads(ret[1])[0].get("result")
        assert result == "aeoyz"

        d = {"times" : 1, "scan_edges" : True}
        js = json.dumps(d)
        ret = client.callProcedure("CPP", "scan", js, 10, False)
        assert ret[0]
        result = json.loads(json.loads(ret[1])[0].get("result"))
        assert result.get("num_edges") == 28 and result.get("num_vertices") == 21

        ret = client.deleteProcedure("CPP", "scan")
        assert ret[0]
        ret = client.listProcedures("CPP", "any")
        assert ret[0]
        plugins = json.loads(ret[1])
        assert len(plugins) == 1

    @pytest.mark.parametrize("build_so", [BUILDV2OPT], indirect=True)
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_plugin_v2(self, build_so, importor, server, client):
        pagerank_so = BUILDV2OPT.get("so_name")[0]
        ret = client.loadProcedure(pagerank_so, "CPP", "v2_pagerank", "SO", "test plugin", True, "v2")
        assert ret[0]
        shortestpath_so = BUILDV2OPT.get("so_name")[1]
        ret = client.loadProcedure(shortestpath_so, "CPP", "v2_test_path", "SO", "test plugin", True, "v2")
        assert ret[0]
        ret = client.callCypher("MATCH (a:Person {name:\"Christopher Nolan\"}), (b:Person {name: \"Corin Redgrave\"}) "
                                "CALL plugin.cpp.v2_test_path(a, b) YIELD length, nodeIds "
                                "RETURN length")
        assert ret[0]
        result = json.loads(ret[1])[0].get("length")
        assert result == 5
        ret = client.callCypher("CALL plugin.cpp.v2_pagerank(10) "
                                "YIELD node, weight WITH node, weight "
                                "RETURN MAX(weight)")
        assert ret[0]
        result = json.loads(ret[1])[0].get("MAX(weight)")
        import math
        assert math.isclose(result, 0.07308246478538732, rel_tol=1e-5)


    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_password(self, importor, server, client):
        ret = client.callCypher("CALL dbms.security.createUser('test_user1', 'pwd1')")
        assert ret[0]
        ret = client.callCypher("CALL dbms.security.addUserRoles('test_user1', ['admin'])")
        assert ret[0]
        test_client = liblgraph_client_python.client("127.0.0.1:9092", "test_user1", "pwd1")
        ret = test_client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        values = json.loads(ret[1])
        assert len(values) == 21

        ret = client.callCypher("CALL dbms.security.changeUserPassword('test_user1','modpwd2')")
        assert ret[0]
        ret = test_client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert not ret[0] # client auto logout after changing password

        test_client2 = liblgraph_client_python.client("127.0.0.1:9092", "test_user1", "modpwd2")
        ret = test_client2.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        values = json.loads(ret[1])
        assert len(values) == 21

        ret = test_client2.callCypher("CALL dbms.security.changePassword('modpwd2','modagainpwd3')")
        assert ret[0]
        ret = test_client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert not ret[0] # client auto logout after changing password
        with pytest.raises(RuntimeError) as exception:
            test_client3 = liblgraph_client_python.client("127.0.0.1:9092", "test_user1", "modpwd2")
        assert exception.typename == "RuntimeError"

        test_client3 = liblgraph_client_python.client("127.0.0.1:9092", "test_user1", "modagainpwd3")
        ret = test_client3.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        values = json.loads(ret[1])
        assert len(values) == 21
        test_client.logout()
        test_client2.logout()
        test_client3.logout()

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_import_file(self, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0
        ret = client.importSchemaFromFile("./data/yago/yago.conf")
        assert ret[0]
        ret = client.callCypher("CALL db.vertexLabels()")
        assert ret[0]
        result = json.loads(ret[1])
        assert len(result) == 3

        ret = client.importDataFromFile("./data/yago/yago.conf", ",")
        assert ret[0]
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 21

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_import_content(self, server, client):
        ret = client.callCypher("MATCH (n) RETURN n LIMIT 100", "default")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 0
        ret = client.importSchemaFromContent(IMPORTCONTENT.get("schema"))
        assert ret[0]
        ret = client.callCypher("CALL db.vertexLabels()")
        assert ret[0]
        result = json.loads(ret[1])
        assert len(result) == 3
        ret = client.callCypher("CALL db.edgeLabels()")
        assert ret[0]
        result = json.loads(ret[1])
        assert len(result) == 7

        ret = client.importDataFromContent(IMPORTCONTENT.get("person_desc"), IMPORTCONTENT.get("person"), ",")
        assert ret[0]
        ret = client.callCypher("match (m:Person) return count(m)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(m)") == 13

        ret = client.importDataFromContent(IMPORTCONTENT.get("city_desc"), IMPORTCONTENT.get("city"), ",")
        assert ret[0]
        ret = client.callCypher("match (m:City) return count(m)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(m)") == 3

        ret = client.importDataFromContent(IMPORTCONTENT.get("film_desc"), IMPORTCONTENT.get("film"), ",")
        assert ret[0]
        ret = client.callCypher("match (m:Film) return count(m)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(m)") == 5

        ret = client.importDataFromContent(IMPORTCONTENT.get("has_child_desc"), IMPORTCONTENT.get("has_child"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:HAS_CHILD]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 7

        ret = client.importDataFromContent(IMPORTCONTENT.get("married_desc"), IMPORTCONTENT.get("married"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:MARRIED]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 4

        ret = client.importDataFromContent(IMPORTCONTENT.get("born_in_desc"), IMPORTCONTENT.get("born_in"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:BORN_IN]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 6

        ret = client.importDataFromContent(IMPORTCONTENT.get("directed_desc"), IMPORTCONTENT.get("directed"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:DIRECTED]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 1

        ret = client.importDataFromContent(IMPORTCONTENT.get("wrote_desc"), IMPORTCONTENT.get("wrote"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 2

        ret = client.importDataFromContent(IMPORTCONTENT.get("acted_in_desc"), IMPORTCONTENT.get("acted_in"), ",")
        assert ret[0]
        ret = client.callCypher("match (n)-[r:ACTED_IN]->(m) return count(r)")
        assert ret[0]
        res = json.loads(ret[1])[0]
        assert res.get("count(r)") == 8

        ret = client.callCypher("call dbms.meta.count()")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 2
        assert res[0]['number'] == 21
        assert res[1]['number'] == 28
        ret = client.callCypher("call dbms.meta.countDetail()")
        assert ret[0]
        assert len(json.loads(ret[1])) == 10
        ret = client.callCypher("call dbms.meta.refreshCount()")
        assert ret[0]
        ret = client.callCypher("call dbms.meta.count()")
        assert ret[0]
        res = json.loads(ret[1])
        assert len(res) == 2
        assert res[0]['number'] == 21
        assert res[1]['number'] == 28
        ret = client.callCypher("call dbms.meta.countDetail()")
        assert ret[0]
        assert len(json.loads(ret[1])) == 10
