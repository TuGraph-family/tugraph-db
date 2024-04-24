import sys
import importlib
sys.path.append("./algo/")
sys.path.append("./")
from lgraph_db_python import *

def python_embed_main(algo):
    galaxy = PyGalaxy(sys.argv[2])
    galaxy.SetCurrentUser("admin", "73@TuGraph")
    db = galaxy.OpenGraph("default", False)
    res = algo.Process(db, "{}".encode('utf-8'))
    print(res)
    del db
    del galaxy

if __name__ == "__main__":
    algo = importlib.import_module(sys.argv[1])
    python_embed_main(algo)