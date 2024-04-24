import logging
import pytest
import json
import os
import time


SCHEMA = '''{"schema": [{"label": "node","type": "VERTEX","properties": [{"name": "field_one","type": "INT64"},{"name" : "field_two", "type":"INT64"}],"primary": "field_one"}],"files": [{"path": "http_server_test_data_import.csv","header": 0,"format": "CSV","label": "node","columns": ["field_one", "field_two"]}]}'''

ESCHEMA = '''{"schema": [{"label": "node","type": "VERTEX","properties": [{"name": "field_one","type": "INT64"},{"name" : "field_two", "type":"INT64"}],"primary": "field_one"}],"files": [{"path": "http_server_test_incorrect_data_import.csv","header": 0,"format": "CSV","label": "node","columns": ["field_one", "field_two"]}]}'''

@pytest.fixture(scope="function")
def correct_data_file():
    try:
        with open("http_server_test_data_import.csv", "w+") as f:
            for i in range (0, 1024*1024*3):
                line = str(i) + " , " + str(1024*1024*3 - (i + 1)) + "\n"
                f.write(line)
    except Exception as e :
        log.info(e)
    yield None
    os.remove("http_server_test_data_import.csv")

@pytest.fixture(scope="function")
def incorrect_data_file():
    try:
        with open("http_server_test_incorrect_data_import.csv", "w+") as f:
            for i in range (0, 1024*1024*3):
                if i == 1024*1024*1:
                    line = str(i) + " , " + str(1024*1024*3 - (i + 1)) + " , " + "error" + "\n"
                else:
                    line = str(i) + " , " + str(1024*1024*3 - (i + 1)) + "\n"
                f.write(line)
    except Exception as e :
        log.info(e)
    yield None
    os.remove("http_server_test_incorrect_data_import.csv")


log = logging.getLogger(__name__)

class TestHttpServer:

    SERVEROPT = {"cmd":"./lgraph_server -c lgraph_standalone.json --directory ./testdb --port 27070 --rpc_port 27071 --log_dir '' ",
                 "cleanup_dir":["./testdb"]}
    CLIENTOPT = {"host":"http://127.0.0.1:27071/LGraphHttpService/Query/", "user":"admin", "password":"73@TuGraph"}


    def read_schema(self, filename):
        try:
            with open(filename, "r") as f:
                return json.load(f)
        except Exception as e :
            log.info(e)
        return None

    def get_file_size(self, file_name):
        try:
            size = os.path.getsize(file_name)
            return size
        except Exception as e :
            log.info(e)
        return None

    def file_exists(self, file_name):
        try:
            return os.path.exists(file_name)
        except Exception as e :
            log.info(e)
        return False

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_import_movie(self, server, rest_client):
        file_name = "./movie/import.json"
        schema = self.read_schema(file_name)
        assert schema != None
        rest_client.import_schema("default", schema)
        res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(res) == 5

        rest_client.upload_file("./movie/raw_data/edge_acted_in.csv")
        res = self.file_exists("./testdb/upload_files/admin/movie/raw_data/edge_acted_in.csv")
        assert res == True

        rest_client.check_file_with_size("movie/raw_data/edge_acted_in.csv", str(self.get_file_size("./movie/raw_data/edge_acted_in.csv")))
        res = self.file_exists("./testdb/upload_files/admin/movie/raw_data/edge_acted_in.csv")
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_directed.csv")
        res = self.file_exists("./testdb/upload_files/admin/movie/raw_data/edge_directed.csv")
        assert res == True

        res = rest_client.check_file_with_size("movie/raw_data/edge_directed.csv", str(self.get_file_size("./movie/raw_data/edge_directed.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_has_genre.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_has_genre.csv", str(self.get_file_size("./movie/raw_data/edge_has_genre.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_has_keyword.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_has_keyword.csv", str(self.get_file_size("./movie/raw_data/edge_has_keyword.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_is_friend.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_is_friend.csv", str(self.get_file_size("./movie/raw_data/edge_is_friend.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_produce.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_produce.csv", str(self.get_file_size("./movie/raw_data/edge_produce.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_rate.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_rate.csv", str(self.get_file_size("./movie/raw_data/edge_rate.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/edge_write.csv")
        res = rest_client.check_file_with_size("movie/raw_data/edge_write.csv", str(self.get_file_size("./movie/raw_data/edge_write.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/vertex_genre.csv")
        res = rest_client.check_file_with_size("movie/raw_data/vertex_genre.csv", str(self.get_file_size("./movie/raw_data/vertex_genre.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/vertex_keyword.csv")
        res = rest_client.check_file_with_size("movie/raw_data/vertex_keyword.csv", str(self.get_file_size("./movie/raw_data/vertex_keyword.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/vertex_movie.csv")
        res = rest_client.check_file_with_size("movie/raw_data/vertex_movie.csv", str(self.get_file_size("./movie/raw_data/vertex_movie.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/vertex_person.csv")
        res = rest_client.check_file_with_size("movie/raw_data/vertex_person.csv", str(self.get_file_size("./movie/raw_data/vertex_person.csv")))
        assert res == True

        rest_client.upload_file("./movie/raw_data/vertex_user.csv")
        res = rest_client.check_file_with_size("movie/raw_data/vertex_user.csv", str(self.get_file_size("./movie/raw_data/vertex_user.csv")))
        assert res == True
        log.info(schema)
        res = rest_client.import_data("default", schema, ",", False, "0", "", "1")
        time.sleep(3)

        res = rest_client.import_progress(res)
        time.sleep(3)
        log.info(res)
        assert res["state"] == "1" or res["state"] == "2"
        res = rest_client.call_cypher("default", "MATCH (n) return COUNT(n)")

        assert res[0]["COUNT(n)"] == 4225

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_upload_file(self, server, rest_client, correct_data_file, incorrect_data_file):
        rest_client.upload_file("./http_server_test_data_import.csv")
        rest_client.upload_file("./http_server_test_incorrect_data_import.csv")
        res = rest_client.check_file_with_size("./http_server_test_data_import.csv", self.get_file_size("./http_server_test_data_import.csv"))
        assert( res == True)
        res = rest_client.check_file_with_size("./http_server_test_incorrect_data_import.csv", self.get_file_size("./http_server_test_incorrect_data_import.csv"))
        assert( res == True)


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_specified_files(self, server, rest_client, correct_data_file, incorrect_data_file):
        rest_client.upload_file("./http_server_test_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == True)
        rest_client.upload_file("./http_server_test_incorrect_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == True)
        rest_client.delete_specified_files("./http_server_test_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == False)
        res = rest_client.delete_specified_files("./http_server_test_incorrect_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csvv")
        assert (res == False)
        res = rest_client.delete_specified_files("./http_server_test_data_import.csv")
        assert (res == "./http_server_test_data_import.csv not exists")

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_delete_specified_user_files(self, server, rest_client, correct_data_file, incorrect_data_file):
        rest_client.upload_file("./http_server_test_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == True)
        rest_client.upload_file("./http_server_test_incorrect_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == True)
        res = self.file_exists("./testdb/upload_files/admin")
        assert (res == True)
        rest_client.delete_specified_user_files("admin")
        res = self.file_exists("./testdb/upload_files/admin")
        assert (res == False)

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_delete_all_user_files(self, server, rest_client, correct_data_file, incorrect_data_file):
        rest_client.upload_file("./http_server_test_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == True)
        rest_client.upload_file("./http_server_test_incorrect_data_import.csv")
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == True)
        res = self.file_exists("./testdb/upload_files")
        assert (res == True)
        rest_client.delete_all_user_files()
        res = self.file_exists("./testdb/upload_files")
        assert (res == False)

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_refresh_token(self, server, rest_client):
        r = rest_client.refresh_token()
        assert (isinstance(r, str))


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_import_correct_data(self, server, rest_client, correct_data_file):
        rest_client.upload_file("http_server_test_data_import.csv")
        res = rest_client.check_file_with_size("http_server_test_data_import.csv", self.get_file_size("http_server_test_data_import.csv"))
        assert( res == True)
        rest_client.import_schema("default", json.loads(SCHEMA))
        res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(res) == 1
        task_id = rest_client.import_data("default", json.loads(SCHEMA), ",", False, "0", "", "1")
        while True:
            time.sleep(5)
            progress = rest_client.import_progress(task_id)
            assert progress["state"] == "1" or progress["state"] == "2"
            if progress["state"] == "2":
                break
        res = rest_client.call_cypher("default", "MATCH (n) return COUNT(n)")
        assert res[0]['COUNT(n)'] == 3145728


    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_import_incorrect_data_with_continue_on_error(self, server, rest_client, incorrect_data_file, correct_data_file):
        rest_client.import_schema("default", json.loads(SCHEMA))
        res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(res) == 1

        rest_client.upload_file("http_server_test_incorrect_data_import.csv")
        res = rest_client.check_file_with_size("http_server_test_incorrect_data_import.csv", self.get_file_size("http_server_test_incorrect_data_import.csv"))
        assert res == True

        task_id = rest_client.import_data("default", json.loads(ESCHEMA), ",", True, "0", "", "1")
        while True:
            time.sleep(5)
            progress = rest_client.import_progress(task_id)
            assert progress["state"] == "1" or progress["state"] == "2"
            if progress["state"] == "2":
                break
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == False)

        rest_client.upload_file("http_server_test_data_import.csv")
        res = rest_client.check_file_with_size("http_server_test_data_import.csv", self.get_file_size("http_server_test_data_import.csv"))
        assert res == True
        task_id = rest_client.import_data("default", json.loads(SCHEMA), ",", True, "0", "", "1")
        while True:
            time.sleep(5)
            progress = rest_client.import_progress(task_id)
            assert progress["state"] == "1" or progress["state"] == "2"
            if progress["state"] == "2":
                break
        res = rest_client.call_cypher("default", "MATCH (n) return COUNT(n)")
        assert res[0]['COUNT(n)'] == 3145728
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == False)
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == False)

    @pytest.mark.parametrize("server", [SERVEROPT], indirect=True)
    @pytest.mark.parametrize("rest_client", [CLIENTOPT], indirect=True)
    def test_import_incorrect_data(self, server, rest_client, incorrect_data_file, correct_data_file):
        rest_client.import_schema("default", json.loads(SCHEMA))
        res = rest_client.call_cypher("default", "CALL db.vertexLabels()")
        assert len(res) == 1

        rest_client.upload_file("http_server_test_incorrect_data_import.csv")
        res = rest_client.check_file_with_size("http_server_test_incorrect_data_import.csv", self.get_file_size("http_server_test_incorrect_data_import.csv"))
        assert res == True

        task_id = rest_client.import_data("default", json.loads(ESCHEMA), ",", False, "0", "", "1")
        while True:
            time.sleep(5)
            progress = rest_client.import_progress(task_id)
            assert progress["state"] == "1" or progress["state"] == "3"
            if progress["state"] == "3":
                break
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_incorrect_data_import.csv")
        assert (res == False)

        rest_client.upload_file("http_server_test_data_import.csv")
        res = rest_client.check_file_with_size("http_server_test_data_import.csv", self.get_file_size("http_server_test_data_import.csv"))
        assert res == True
        task_id = rest_client.import_data("default", json.loads(SCHEMA), ",", True, "0", task_id, "1")
        while True:
            time.sleep(5)
            progress = rest_client.import_progress(task_id)
            assert progress["state"] == "1" or progress["state"] == "2"
            if progress["state"] == "2":
                break
        res = rest_client.call_cypher("default", "MATCH (n) return COUNT(n)")
        assert res[0]['COUNT(n)'] == 3145728
        res = self.file_exists("./testdb/upload_files/admin/http_server_test_data_import.csv")
        assert (res == False)

    # TODO(anyone): add more interface tests
