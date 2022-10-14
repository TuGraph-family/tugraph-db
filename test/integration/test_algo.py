
import pytest
import logging

log = logging.getLogger(__name__)

CUCMD = {"cmd":"./unit_test_cu -t RPC"}

ERCMD = {"cmd":"./unit_test_er -t RPC"}

class TestExec():

    BFSEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/bfs_embed ./testdb",
        "result" : ["found_vertices = 3829"]
    }

    IMPORTOPT = {
        "cmd" : "./lgraph_import -c ./data/algo/fb.conf -d ./testdb --overwrite 1",
        "cleanup_dir" : ["./testdb", "./.import_tmp"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [BFSEMBEDOPT], indirect=True)
    def test_exec_bfs_embed(self, importor, algo):
        pass


    BFSSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/bfs_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["found_vertices = 3829"]
    }

    @pytest.mark.parametrize("algo", [BFSSTANDOPT], indirect=True)
    def test_exec_bfs_standalone(self, algo):
        pass

    PGEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_embed ./testdb",
        "result" : [" pr[1911] = 0.004525"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [PGEMBEDOPT], indirect=True)
    def test_exec_pagerank_embed(self, importor, algo):
        pass


    PGSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : [" pr[1911] = 0.004525"]
    }
    @pytest.mark.parametrize("algo", [PGSTANDOPT], indirect=True)
    def test_exec_pagerank_standalone(self, algo):
        pass


    SSSPEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/sssp_embed ./testdb",
        "result" : ["]=5"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SSSPEMBEDOPT], indirect=True)
    def test_exec_sssp_embed(self, importor, algo):
        pass


    SSSPSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/sssp_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["]=5"]
    }
    @pytest.mark.parametrize("algo", [SSSPSTANDOPT], indirect=True)
    def test_exec_sssp_standalone(self, algo):
        pass


    WCCEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/wcc_embed ./testdb",
        "result" : ['''"max_component":4039,"num_components":1''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [WCCEMBEDOPT], indirect=True)
    def test_exec_wcc_embed(self, importor, algo):
        pass


    WCCSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/wcc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["max_component = 4039", "num_components = 1"]
    }
    @pytest.mark.parametrize("algo", [WCCSTANDOPT], indirect=True)
    def test_exec_wcc_standalone(self, algo):
        pass

    LCCEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lcc_embed ./testdb",
        "result" : ['''"average_clco":0.60554''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LCCEMBEDOPT], indirect=True)
    def test_exec_lcc_embed(self, importor, algo):
        pass

    LCCSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lcc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["average_clco is: 0.60554"]
    }
    @pytest.mark.parametrize("algo", [LCCSTANDOPT], indirect=True)
    def test_exec_lcc_standalone(self, algo):
        pass

    LPAEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lpa_embed ./testdb",
        "result" : ['''"modularity":0.770773''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LPAEMBEDOPT], indirect=True)
    def test_exec_lpa_embed(self, importor, algo):
        pass

    LPASTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lpa_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["modularity: 0.770773"]
    }
    @pytest.mark.parametrize("algo", [LPASTANDOPT], indirect=True)
    def test_exec_lpa_standalone(self, algo):
        pass    
