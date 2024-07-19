import logging
import pytest
import json

log = logging.getLogger(__name__)

SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093",
             "cleanup_dir":[]}

SERVEROPT_1 = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 7073 --rpc_port 9093",
             "cleanup_dir":["./testdb"]}

CLIENTOPT = {"host":"127.0.0.1:9093", "user":"admin", "password":"73@TuGraph"}

class TestCypher:


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_config_update_one(self, server, client):
        ret = client.callCypher("CALL dbms.config.update({enable_ip_check:false, durable:true, optimistic_txn:false, enable_audit_log:true})", "default")
        assert ret[0]

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_config_list_one(self, server, client):
        ret = client.callCypher("CALL dbms.config.list()", "default")
        configs = json.loads(ret[1])
        dict = {}
        for config in configs:
            dict[config["name"]] = config["value"]
        assert(dict['enable_ip_check'] == False and
               dict['durable'] == True and
               dict['optimistic_txn'] == False and
               dict['enable_audit_log'] == True)


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_config_update_two(self, server, client):
        ret = client.callCypher("CALL dbms.config.update({enable_ip_check:true, durable:false, optimistic_txn:true, enable_audit_log:false})", "default")
        assert ret[0]

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_config_list_two(self, server, client):
        ret = client.callCypher("CALL dbms.config.list()", "default")
        configs = json.loads(ret[1])
        dict = {}
        for config in configs:
            dict[config["name"]] = config["value"]
        assert(dict['enable_ip_check'] == True and
               dict['durable'] == False and
               dict['optimistic_txn'] == True and
               dict['enable_audit_log'] == False)

    @pytest.mark.parametrize("server", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_persisted_after_reboot(self, server, client):
        ret = client.callCypher("CALL dbms.config.list()", "default")
        configs = json.loads(ret[1])
        dict = {}
        for config in configs:
            dict[config["name"]] = config["value"]
        assert(dict['enable_ip_check'] == True and
               dict['durable'] == False and
               dict['optimistic_txn'] == True and
               dict['enable_audit_log'] == False)

    @pytest.mark.parametrize("server", [SERVEROPT_1], indirect=True)
    @pytest.mark.parametrize("client", [CLIENTOPT], indirect=True)
    def test_issue406(self, server, client):
        client.callCypher("CALL db.createVertexLabel('person', 'id', 'id', 'INT64', false, 'name', 'string', true, 'born', 'int32', true, 'poster_image', 'string', true)")
        client.callCypher("CREATE (n:person {id:1, name:'Tom Hanks', born:1956, poster_image:'http://image.com'})")
        client.callCypher("CREATE (n:person {id:2, name:'2', born:1962, poster_image:'http://image.com'})")
        client.callCypher("CREATE (n:person {id:3, name:'309485009821345068724781056', born:1962, poster_image:'http://image.com'})")
        ret = client.callCypher("CREATE (n:person {id:1125899906842624, name:'0023', born:1962})")
        ret = client.callCypher("MATCH (n) RETURN n", "default")
        items = json.loads(ret[1])
        assert(len(items) == 4 and
               items[0]["n"]["properties"]["name"] == "Tom Hanks" and
               items[1]["n"]["properties"]["name"] == "2" and
               items[2]["n"]["properties"]["name"] == "309485009821345068724781056" and
               items[3]["n"]["properties"]["name"] == "0023")
        assert(items[3]["n"]["properties"]["id"] == 1125899906842624 and
               items[3]["n"]["properties"]["poster_image"] == None)
