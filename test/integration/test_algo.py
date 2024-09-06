import pytest
import logging

log = logging.getLogger(__name__)

CUCMD = {"cmd":"./unit_test_cu -t RPC"}

ERCMD = {"cmd":"./unit_test_er -t RPC"}


class TestExec:
    APSPEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/apsp_embed ./testdb",
        "result": ["max_distance is distance[1412,3143] = 17"]
    }

    IMPORTOPT = {
        "cmd": "./lgraph_import -c ./data/algo/fb.conf -d ./testdb --overwrite 1 && mkdir louvain_output",
        "cleanup_dir": ["./testdb", "./.import_tmp"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [APSPEMBEDOPT], indirect=True)
    def test_exec_apsp_embed(self, importor, algo):
        pass

    APSPSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/apsp_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max_distance is distance[1412,3143] = 17"]
    }

    @pytest.mark.parametrize("algo", [APSPSTANDOPT], indirect=True)
    def test_exec_apsp_standalone(self, algo):
        pass

    BFSEMBEDOPT = {
        "cmd" : "OMP_NUM_THREADS=6 algo/bfs_embed ./testdb",
        "result" : ["found_vertices = 3829"]
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

    BCEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/bc_embed ./testdb",
        "result": ['''"max_score":1.0,"max_score_vid":1465''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [BCEMBEDOPT], indirect=True)
    def test_exec_bc_embed(self, importor, algo):
        pass

    BCSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/bc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max score is: score[1465]=1"]
    }

    @pytest.mark.parametrize("algo", [BCSTANDOPT], indirect=True)
    def test_exec_bc_standalone(self, algo):
        pass

    CLCEEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/clce_embed ./testdb",
        "result": ['''"max_length":1.0,"max_length_vi":1684''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [CLCEEMBEDOPT], indirect=True)
    def test_exec_clce_embed(self, importor, algo):
        pass

    CLCESTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/clce_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max_score is score[1684] = 1.0"]
    }

    @pytest.mark.parametrize("algo", [CLCESTANDOPT], indirect=True)
    def test_exec_clce_standalone(self, algo):
        pass

    CNEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/cn_embed ./testdb",
        "result": ['''"cn_list":[[0,1,16.0],[1,972,0.0],[101,202,1.0],[1000,4000,0.0]]''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [CNEMBEDOPT], indirect=True)
    def test_exec_cn_embed(self, importor, algo):
        pass

    CNSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/cn_standalone --type text --input_dir ./data/algo/fb_weighted --search_dir ./data/algo/search_dir/ --make_symmetric 1",
        "result": ["cn(0,1) = 16", "cn(1,972) = 0", "cn(101,202) = 1", "cn(1000,4000) = 0"]
    }

    @pytest.mark.parametrize("algo", [CNSTANDOPT], indirect=True)
    def test_exec_cn_standalone(self, algo):
        pass

    DCEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/dc_embed ./testdb",
        "result": ['''"graph_dc":1.92524''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [DCEMBEDOPT], indirect=True)
    def test_exec_dc_embed(self, importor, algo):
        pass

    DCSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/dc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["graph_dc:1.92524"]
    }

    @pytest.mark.parametrize("algo", [DCSTANDOPT], indirect=True)
    def test_exec_dc_standalone(self, algo):
        pass

    DEEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/de_embed ./testdb",
        "result": ['''"max_diamension":9''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [DEEMBEDOPT], indirect=True)
    def test_exec_de_embed(self, importor, algo):
        pass

    DESTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/de_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max_diamension:9"]
    }

    @pytest.mark.parametrize("algo", [DESTANDOPT], indirect=True)
    def test_exec_de_standalone(self, algo):
        pass

    FASTTRIANGLEEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/fast_triangle_counting_embed ./testdb",
        "result": ['''"discovered_triangles":1612010''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [FASTTRIANGLEEMBEDOPT], indirect=True)
    def test_exec_fast_triangle_embed(self, importor, algo):
        pass

    FASTTRIANGLESTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/fast_triangle_counting_standalone --type text --input_dir ./data/algo/fb_unweighted --make_symmetric 1",
        "result": ["discovered 1612010 triangles"]
    }

    @pytest.mark.parametrize("algo", [FASTTRIANGLESTANDOPT], indirect=True)
    def test_exec_fast_triangle_standalone(self, algo):
        pass

    HITSEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/hits_embed ./testdb",
        "result": ["max authority value is authority[2604] = 0.11527", "max hub value is hub[1912] = 0.13993"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [HITSEMBEDOPT], indirect=True)
    def test_exec_hits_embed(self, importor, algo):
        pass

    HITSSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/hits_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max authority value is authority[2604] = 0.11527", "max hub value is hub[1912] = 0.13993"]
    }

    @pytest.mark.parametrize("algo", [HITSSTANDOPT], indirect=True)
    def test_exec_hits_standalone(self, algo):
        pass

    JIEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ji_embed ./testdb",
        "result": ['''"ji_list":[["0","1",0.04597701149425287],["1","972",0.0],["101","202",0.045454545454545456]]''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [JIEMBEDOPT], indirect=True)
    def test_exec_ji_embed(self, importor, algo):
        pass

    JISTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ji_standalone --type text --input_dir ./data/algo/fb_weighted --search_dir ./data/algo/search_dir/ --make_symmetric 1",
        "result": ["ji(0,1) = 0.045977", "ji(1,972) = 0.000000", "ji(101,202) = 0.04545"]
    }

    @pytest.mark.parametrize("algo", [JISTANDOPT], indirect=True)
    def test_exec_ji_standalone(self, algo):
        pass

    KCLIQUESEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/kcliques_embed ./testdb",
        "result": ['''discovered 1612010 3-cliques''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [KCLIQUESEMBEDOPT], indirect=True)
    def test_exec_kcliques_embed(self, importor, algo):
        pass

    KCLIQUESSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/kcliques_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["found 1612010 3-cliques"]
    }

    @pytest.mark.parametrize("algo", [KCLIQUESSTANDOPT], indirect=True)
    def test_exec_kcliques_standalone(self, algo):
        pass

    KCOREEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/kcore_embed ./testdb",
        "result": ['''"num_result_vertices":2987''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [KCOREEMBEDOPT], indirect=True)
    def test_exec_kcore_embed(self, importor, algo):
        pass

    KCORESTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/kcore_standalone --type text --input_dir ./data/algo/fb_weighted --make_symmetric 1",
        "result": ["number of 10-core vertices: 2987"]
    }

    @pytest.mark.parametrize("algo", [KCORESTANDOPT], indirect=True)
    def test_exec_kcore_standalone(self, algo):
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

    KTRUSSEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ktruss_embed ./testdb",
        "result": ['''"left_edges":88156''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [KTRUSSEMBEDOPT], indirect=True)
    def test_exec_ktruss_embed(self, importor, algo):
        pass

    KTRUSSSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ktruss_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["last_edge:88156"]
    }

    @pytest.mark.parametrize("algo", [KTRUSSSTANDOPT], indirect=True)
    def test_exec_ktruss_standalone(self, algo):
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

    LEIDENEMBEDOPT = {
        "cmd" : "algo/leiden_embed ./testdb",
        "result" : ['''final Q is 0.9''']
    }
    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LEIDENEMBEDOPT], indirect=True)
    def test_exec_leiden_embed(self, importor, algo):
        pass

    LEIDENSTANDOPT = {
        "cmd" : "algo/leiden_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result" : ["final Q is 0.9"]
    }
    @pytest.mark.parametrize("algo", [LEIDENSTANDOPT], indirect=True)
    def test_exec_leiden_standalone(self, algo):
        pass

    LCIMPORTOPT = {
        "cmd": "OMP_NUM_THREADS=6 ./lgraph_import -c ./data/algo/lc.conf -d ./locate_cycle_db --overwrite 1",
        "cleanup_dir": ["./locate_cycle_db", "./.import_tmp"]
    }

    LCEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/locate_cycle_embed ./locate_cycle_db",
        "result": ['''"num_rings":4''']
    }

    @pytest.mark.parametrize("importor", [LCIMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LCEMBEDOPT], indirect=True)
    def test_exec_lc_embed(self, importor, algo):
        pass

    LCSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/locate_cycle_standalone --type text --input_dir ./data/algo/locate_cycle_unweighted",
        "result": ["the num of rings is: 4"]
    }

    @pytest.mark.parametrize("algo", [LCSTANDOPT], indirect=True)
    def test_exec_lc_standalone(self, algo):
        pass

    LOUVAINEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/louvain_embed ./testdb",
        "result": ['''"modularity":0.83''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [LOUVAINEMBEDOPT], indirect=True)
    def test_exec_louvain_embed(self, importor, algo):
        pass

    LOUVAINSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/louvain_standalone --type text --input_dir ./data/algo/fb_weighted --output_dir louvain_output/",
        "result": ["Q = 0.83"]
    }

    @pytest.mark.parametrize("algo", [LOUVAINSTANDOPT], indirect=True)
    def test_exec_louvain_standalone(self, algo):
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

    MISEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/mis_embed ./testdb",
        "result": ['''"MIS_size":499''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [MISEMBEDOPT], indirect=True)
    def test_exec_mis_embed(self, importor, algo):
        pass

    MISSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/mis_standalone --type text --input_dir ./data/algo/fb_unweighted --make_symmetric 1",
        "result": ["|mis|=499"]
    }

    @pytest.mark.parametrize("algo", [MISSTANDOPT], indirect=True)
    def test_exec_mis_standalone(self, algo):
        pass

    MOTIFEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/motif_embed ./testdb",
        "result": ['''[6,57512]''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [MOTIFEMBEDOPT], indirect=True)
    def test_exec_motif_embed(self, importor, algo):
        pass

    MOTIFSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/motif_standalone --type text --input_dir ./data/algo/fb_unweighted --value_k 3 --vertices_ids 0,1",
        "result": ["[6,57512]"]
    }

    @pytest.mark.parametrize("algo", [MOTIFSTANDOPT], indirect=True)
    def test_exec_motif_standalone(self, algo):
        pass

    MSSPEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/mssp_embed ./testdb",
        "result": ['''"max_distance_val":5.0''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [MSSPEMBEDOPT], indirect=True)
    def test_exec_mssp_embed(self, importor, algo):
        pass

    MSSPSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/mssp_standalone --type text --input_dir ./data/algo/fb_weighted --roots_dir ./data/algo/roots_dir/",
        "result": ["max distance is: 5.0"]
    }

    @pytest.mark.parametrize("algo", [MSSPSTANDOPT], indirect=True)
    def test_exec_mssp_standalone(self, algo):
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

    PPREMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ppr_embed ./testdb",
        "result": ["pr[0] = 0.15"]
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [PPREMBEDOPT], indirect=True)
    def test_exec_ppr_embed(self, importor, algo):
        pass

    PPRSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/ppr_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["pr[0] = 0.15"]
    }

    @pytest.mark.parametrize("algo", [PPRSTANDOPT], indirect=True)
    def test_exec_ppr_standalone(self, algo):
        pass

    SCCEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/scc_embed ./testdb",
        "result": ['''"max_component":1,"num_components":4039''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SCCEMBEDOPT], indirect=True)
    def test_exec_scc_embed(self, importor, algo):
        pass

    SCCSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/scc_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["num_components = 4039"]
    }

    @pytest.mark.parametrize("algo", [SCCSTANDOPT], indirect=True)
    def test_exec_scc_standalone(self, algo):
        pass

    SLPAEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/slpa_embed ./testdb",
        "result": ['''"modularity":0.8''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SLPAEMBEDOPT], indirect=True)
    def test_exec_slpa_embed(self, importor, algo):
        pass

    SLPASTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/slpa_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["modularity: 0.8"]
    }

    @pytest.mark.parametrize("algo", [SLPASTANDOPT], indirect=True)
    def test_exec_slpa_standalone(self, algo):
        pass

    SPSPEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/spsp_embed ./testdb",
        "result": ['''"length_list":[[0,1,1],[1,972,10]]''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SPSPEMBEDOPT], indirect=True)
    def test_exec_spsp_embed(self, importor, algo):
        pass

    SPSPSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/spsp_standalone --type text --input_dir ./data/algo/fb_weighted --search_dir ./data/algo/search_dir/",
        "result": ["psp(0,1) = 1",
                   "psp(1,972) = 10"]
    }

    @pytest.mark.parametrize("algo", [SPSPSTANDOPT], indirect=True)
    def test_exec_spsp_standalone(self, algo):
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

    SUBFRAPHISOMORPHISMEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/subgraph_isomorphism_embed ./testdb",
        "result": ['''"match_subgraph_num":1612010''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SUBFRAPHISOMORPHISMEMBEDOPT], indirect=True)
    def test_exec_subgraph_isomorphism_embed(self, importor, algo):
        pass

    SUBFRAPHISOMORPHISMSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/subgraph_isomorphism_standalone --type text --input_dir ./data/algo/fb_unweighted --query [[1,2],[2],[]]",
        "result": ["match 1612010 subgraph"]
    }

    @pytest.mark.parametrize("algo", [SUBFRAPHISOMORPHISMSTANDOPT], indirect=True)
    def test_exec_subgraph_isomorphism_standalone(self, algo):
        pass

    SYBILRANKEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/sybilrank_embed ./testdb",
        "result": ['''"max_sybilrank":0.0008''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [SYBILRANKEMBEDOPT], indirect=True)
    def test_exec_sybilrank_embed(self, importor, algo):
        pass

    SYBILRANKSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/sybilrank_standalone --type text --input_dir ./data/algo/fb_unweighted --trust_seeds 0",
        "result": ["max rank value is sybilrank[332] = 0.000849"]
    }

    @pytest.mark.parametrize("algo", [SYBILRANKSTANDOPT], indirect=True)
    def test_exec_sybilrank_standalone(self, algo):
        pass

    TRIANGLEEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/triangle_embed ./testdb",
        "result": ['''"discovered_triangles":1612010''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [TRIANGLEEMBEDOPT], indirect=True)
    def test_exec_triangle_embed(self, importor, algo):
        pass

    TRIANGLESTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/triangle_standalone --type text --input_dir ./data/algo/fb_weighted --make_symmetric 1",
        "result": ["discovered 1612010 triangles"]
    }

    @pytest.mark.parametrize("algo", [TRIANGLESTANDOPT], indirect=True)
    def test_exec_triangle_standalone(self, algo):
        pass

    TRUSTRANKEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/trustrank_embed ./testdb",
        "result": ['''"max_trustrank_id":3,"max_trustrank_val":0.05''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [TRUSTRANKEMBEDOPT], indirect=True)
    def test_exec_trustrank_embed(self, importor, algo):
        pass

    TRUSTRANKSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/trustrank_standalone --type text --input_dir ./data/algo/fb_weighted --trustedUser_dir ./data/algo/roots_dir/",
        "result": ["max rank value is [3] = 0.050122"]
    }

    @pytest.mark.parametrize("algo", [TRUSTRANKSTANDOPT], indirect=True)
    def test_exec_trustrank_standalone(self, algo):
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

    WLPAEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/wlpa_embed ./testdb",
        "result": ['''"modularity":0.8''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [WLPAEMBEDOPT], indirect=True)
    def test_exec_wlpa_embed(self, importor, algo):
        pass

    WLPASTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/wlpa_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["modularity: 0.8"]
    }

    @pytest.mark.parametrize("algo", [WLPASTANDOPT], indirect=True)
    def test_exec_wlpa_standalone(self, algo):
        pass

    WPAGERANKEMBEDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/wpagerank_embed ./testdb",
        "result": ['''"max_wpr_vi":1911''']
    }

    @pytest.mark.parametrize("importor", [IMPORTOPT], indirect=True)
    @pytest.mark.parametrize("algo", [WPAGERANKEMBEDOPT], indirect=True)
    def test_exec_wpagerank__embed(self, importor, algo):
        pass

    WPAGERANKSTANDOPT = {
        "cmd": "OMP_NUM_THREADS=6 algo/wpagerank_standalone --type text --input_dir ./data/algo/fb_weighted",
        "result": ["max rank value is [1911] = 0.004525"]
    }

    @pytest.mark.parametrize("algo", [WPAGERANKSTANDOPT], indirect=True)
    def test_exec_wpagerank__standalone(self, algo):
        pass