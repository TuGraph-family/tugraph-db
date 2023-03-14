import pytest
import logging

log = logging.getLogger(__name__)

CUCMD = {"cmd":"./unit_test_cu -t RPC"}

ERCMD = {"cmd":"./unit_test_er -t RPC"}


class TestExec:

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

    BFSIDMAPPING = {
        "cmd" : "OMP_NUM_THREADS=6 algo/bfs_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 1 --root 0a",
        "result" : ["found_vertices = 3829"]
    }

    @pytest.mark.parametrize("algo", [BFSIDMAPPING], indirect=True)
    def test_exec_bfs_idmapping_standalone(self, algo):
        pass

    BFSNOIDMAPPING = {
        "cmd" : "OMP_NUM_THREADS=6 algo/bfs_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 0 --root 0a",
        "result" : ["found_vertices = 3829"]
    }
    @pytest.mark.parametrize("algo", [BFSNOIDMAPPING], indirect=True)
    @pytest.mark.xfail(reason="need id mapping")
    def test_exec_bfs_without_idmapping_standalone(self, algo):
        pass

    BFSPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py bfs ./testdb",
        "result" : ['''"found_vertices": 3829''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [BFSPYTHONEMBEDOPT], indirect=True)
    def test_exec_bfs_python_embed(self, importor, algo):
        pass

    BFSPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py bfs ./data/algo/fb_weighted",
        "result" : ["found_vertices = 3829"]
    }
    @pytest.mark.parametrize("algo", [BFSPYTHONSTANDOPT], indirect=True)
    def test_exec_bfs_python_standalone(self, algo):
        pass


    PGEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_embed ./testdb",
        "result" : [" pr[1911] = 0.009418"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [PGEMBEDOPT], indirect=True)
    def test_exec_pagerank_embed(self, importor, algo):
        pass


    PGSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : [" pr[1911] = 0.009418"]
    }
    @pytest.mark.parametrize("algo", [PGSTANDOPT], indirect=True)
    def test_exec_pagerank_standalone(self, algo):
        pass

    PGIDMAPPINGSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 1",
        "result" : [" pr[1911a] = 0.009418"]
    }
    @pytest.mark.parametrize("algo", [PGIDMAPPINGSTANDOPT], indirect=True)
    def test_exec_idmapping_pagerank_standalone(self, algo):
        pass

    PGNOIDMAPPINGSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/pagerank_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 0",
        "result" : [" pr[1911a] = 0.009418"]
    }
    @pytest.mark.parametrize("algo", [PGNOIDMAPPINGSTANDOPT], indirect=True)
    @pytest.mark.xfail(reason="need id mapping")
    def test_exec_without_idmapping_pagerank_standalone(self, algo):
        pass

    PGPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py pagerank ./testdb",
        "result" : ['''"max_pr": 0.009418''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [PGPYTHONEMBEDOPT], indirect=True)
    def test_exec_pagerank_python_embed(self, importor, algo):
        pass

    PGPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py pagerank ./data/algo/fb_weighted",
        "result" : ["pr[1911] = 0.009418"]
    }
    @pytest.mark.parametrize("algo", [PGPYTHONSTANDOPT], indirect=True)
    def test_exec_pagerank_python_standalone(self, algo):
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

    SSSPIDMAPPING = {
        "cmd" : "OMP_NUM_THREADS=6 algo/sssp_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 1 --root 0a",
        "result" : ["]=5"]
    }

    @pytest.mark.parametrize("algo", [SSSPIDMAPPING], indirect=True)
    def test_exec_sssp_idmapping_standalone(self, algo):
        pass

    SSSPNOIDMAPPING = {
        "cmd" : "OMP_NUM_THREADS=6 algo/sssp_standalone --type text --input_dir ./data/algo/fb_str_weighted --id_mapping 0 --root 0a",
        "result" : ["]=5"]
    }
    @pytest.mark.parametrize("algo", [SSSPNOIDMAPPING], indirect=True)
    @pytest.mark.xfail(reason="need id mapping")
    def test_exec_sssp_without_idmapping_standalone(self, algo):
        pass

    SSSPPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py sssp ./testdb",
        "result" : ['''"max_distance": 5''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SSSPPYTHONEMBEDOPT], indirect=True)
    def test_exec_sssp_python_embed(self, importor, algo):
        pass

    SSSPPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py sssp ./data/algo/fb_weighted",
        "result" : ["] = 5"]
    }
    @pytest.mark.parametrize("algo", [SSSPPYTHONSTANDOPT], indirect=True)
    def test_exec_sssp_python_standalone(self, algo):
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

    WCCPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py wcc ./testdb",
        "result" : ['''"max_component": 4039''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [WCCPYTHONEMBEDOPT], indirect=True)
    def test_exec_wcc_python_embed(self, importor, algo):
        pass

    WCCPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py wcc ./data/algo/fb_weighted",
        "result" : ["max_component = 4039", "num_components = 1"]
    }
    @pytest.mark.parametrize("algo", [WCCPYTHONSTANDOPT], indirect=True)
    def test_exec_wcc_python_standalone(self, algo):
        pass

    LCCEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lcc_embed ./testdb",
        "result" : ['''"average_lcc":0.60554''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LCCEMBEDOPT], indirect=True)
    def test_exec_lcc_embed(self, importor, algo):
        pass

    LCCSTANDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/lcc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["average_lcc is: 0.60554"]
    }
    @pytest.mark.parametrize("algo", [LCCSTANDOPT], indirect=True)
    def test_exec_lcc_standalone(self, algo):
        pass

    LCCPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py lcc ./testdb",
        "result" : ['''"average_lcc": 0.60554''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LCCPYTHONEMBEDOPT], indirect=True)
    def test_exec_lcc_python_embed(self, importor, algo):
        pass

    LCCPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py lcc ./data/algo/fb_weighted",
        "result" : ["average_lcc = 0.60554"]
    }
    @pytest.mark.parametrize("algo", [LCCPYTHONSTANDOPT], indirect=True)
    def test_exec_lcc_python_standalone(self, algo):
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

    LPAPYTHONEMBEDOPT = {
        "cmd": "python3 python_embed.py lpa ./testdb",
        "result" : ['''"max_community": 1015''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LPAPYTHONEMBEDOPT], indirect=True)
    def test_exec_lpa_python_embed(self, importor, algo):
        pass

    LPAPYTHONSTANDOPT = {
        "cmd": "python3 python_standalone.py lpa ./data/algo/fb_weighted",
        "result" : ["max_community = 1015"]
    }
    @pytest.mark.parametrize("algo", [LPAPYTHONSTANDOPT], indirect=True)
    def test_exec_lpa_python_standalone(self, algo):
        pass



    KHOPKTH = {
        "cmd" : "OMP_NUM_THREADS=6 algo/khop_kth_embed ./testdb",
        "result" : ['''"size":3168''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [KHOPKTH], indirect=True)
    def test_exec_khopkth_embed(self, importor, algo):
        pass

    KHOPWITHIN = {
        "cmd" : "OMP_NUM_THREADS=6 algo/khop_within_embed ./testdb",
        "result" : ['''"size":3259''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [KHOPWITHIN], indirect=True)
    def test_exec_khopwithin_embed(self, importor, algo):
        pass
