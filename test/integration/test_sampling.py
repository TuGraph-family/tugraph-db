import pytest
import logging

log = logging.getLogger(__name__)

CUCMD = {"cmd":"./unit_test_cu -t RPC"}

ERCMD = {"cmd":"./unit_test_er -t RPC"}

class TestExec:

    GETDB = {
        "cmd" : "python3 python_sampling.py getdb ./coradb",
        "result" : ["the num of NodeInfo = 2708"]
    }

    IMPORTOPT = {
        "cmd" : "./lgraph_import -c ./data/algo/cora.conf --dir ./coradb --overwrite 1 && ./algo/feature_float_embed ./coradb",
        "cleanup_dir" : ["./testdb", "./.import_tmp"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [GETDB], indirect=True)
    def test_exec_getdb_python_embed(self, importor, algo):
        pass

    EDGESAMPLING = {
        "cmd" : "python3 python_sampling.py edge_sampling ./coradb",
        "result" : ["the num of NodeInfo = 2708"]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [EDGESAMPLING], indirect=True)
    def test_exec_edgesampling_python_embed(self, importor, algo):
        pass

    NEGATIVESAMPLING = {
        "cmd" : "python3 python_sampling.py negative_sampling ./coradb",
        "result" : ["the num of NodeInfo ="]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [NEGATIVESAMPLING], indirect=True)
    def test_exec_negativesampling_python_embed(self, importor, algo):
        pass

    NEIGHBORSAMPLING = {
        "cmd" : "python3 python_sampling.py neighbors_sampling ./coradb",
        "result" : ["the label of 35 is: 6"]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [NEIGHBORSAMPLING], indirect=True)
    def test_exec_neighborsampling_python_embed(self, importor, algo):
        pass

    RANDOMWALK = {
        "cmd" : "python3 python_sampling.py random_walk ./coradb",
        "result" : ["the label of 35 is: 6"]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [RANDOMWALK], indirect=True)
    def test_exec_randomwalk_python_embed(self, importor, algo):
        pass
    
    NODE2VECSAMPLING = {
        "cmd" : "python3 python_sampling.py node2vec_sampling ./coradb",
        "result" : ["the label of 35 is: 6"]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [NODE2VECSAMPLING], indirect=True)
    def test_exec_node2vecsampling_python_embed(self, importor, algo):
        pass