import pytest
import logging

log = logging.getLogger(__name__)

CUCMD = {"cmd":"./unit_test_cu -t RPC"}

ERCMD = {"cmd":"./unit_test_er -t RPC"}

class TestExec:

    TRAINCORA = {
        "cmd" : "python3 train_cora.py",
        "result" : ["The loss value is less than 0.9"]
    }

    IMPORTOPT = {
        "cmd" : "./lgraph_import -c ./data/algo/cora.conf --dir ./coradb --overwrite 1 && ./algo/feature_float_embed ./coradb",
        "cleanup_dir" : ["./coradb", "./.import_tmp"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [TRAINCORA], indirect=True)
    def test_exec_train_cora_python_embed(self, importor, algo):
        pass

    TRAINFULLCORA = {
        "cmd" : "python3 train_full_cora.py",
        "result" : ["The loss value is less than 0.9"]
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [TRAINFULLCORA], indirect=True)
    def test_exec_train_full_cora_python_embed(self, importor, algo):
        pass
    MAPIMPORTOPT = {
        "cmd" : "./lgraph_import -c ./data/algo/mag.conf --dir ./magdb --overwrite 1 && ./algo/feature_float_embed ./magdb",
        "cleanup_dir" : ["./magdb", "./.import_tmp"]
    }

    TRAINFULLMAG = {
        "cmd" : "python3 train_full_mag.py",
        "result" : ["The loss value is less than 5"]
    }
    @pytest.mark.parametrize("importor", [MAPIMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [TRAINFULLMAG], indirect=True)
    def test_exec_train_full_mag_python_embed(self, importor, algo):
        pass

