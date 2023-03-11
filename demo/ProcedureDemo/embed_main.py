from liblgraph_python_api import GraphDB, Galaxy
import json

# --- add python plugin ---
import python.scan_graph as python_plugin
# --- add python plugin ---

if __name__ == "__main__":
    galaxy = Galaxy("lgraph_db")
    galaxy.SetCurrentUser("admin", "73@TuGraph")
    db = galaxy.OpenGraph("default", False)
    res = python_plugin.Process(db, "{\"scan_edges\": true}")
    print(res)
    db.Close()
    galaxy.Close()

