from lgraph_db_python import *
import json

# --- add python plugin ---
import wcc_procedure as python_plugin

# --- add python plugin ---

if __name__ == "__main__":
    galaxy = PyGalaxy("lgraph_db")
    galaxy.SetCurrentUser("admin", "73@TuGraph")
    db = galaxy.OpenGraph("default", False)
    res = python_plugin.Process(db, "{}".encode('utf-8'))
    print(res)
    del db
    del galaxy
