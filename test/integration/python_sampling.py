import sys
sys.path.append("./algo/")
import argparse
import queue
from lgraph_db_python import *
import sys
import importlib

def python_embed_main(algo):
    NodeInfo = []
    EdgeInfo = []
    galaxy = PyGalaxy(sys.argv[2])
    galaxy.SetCurrentUser("admin", "73@TuGraph")
    db = galaxy.OpenGraph("default", False)
    txn = db.CreateReadTxn()
    olapondb = PyOlapOnDB("Empty", db, txn)
    del txn
    if sys.argv[1] == "getdb":
        algo.Process(db, olapondb, 1433, NodeInfo, EdgeInfo)
        print("the num of NodeInfo =", len(NodeInfo[0]))
    if sys.argv[1] == "edge_sampling":
        algo.Process(db, olapondb, 1433, 1, NodeInfo, EdgeInfo)
        print("the num of NodeInfo =", len(NodeInfo[0]))
    if sys.argv[1] == "random_walk":
        algo.Process(db, olapondb, 1433, [163], 2, NodeInfo, EdgeInfo)
        print("the label of 35 is:", NodeInfo[2][0])
    if sys.argv[1] == "neighbors_sampling":
        algo.Process(db, olapondb, 1433, [163], 2, NodeInfo, EdgeInfo)
        print("the label of 35 is:", NodeInfo[2][0])
    if sys.argv[1] == "negative_sampling":
        algo.Process(db, olapondb, 1433, 100, NodeInfo, EdgeInfo)
        print("the num of NodeInfo =")
    if sys.argv[1] == "node2vec_sampling":
        algo.Process(db, olapondb, 1433, 100, NodeInfo, EdgeInfo)
        print("the num of NodeInfo =")
    del db
    del galaxy

if __name__ == "__main__":
    algo = importlib.import_module(sys.argv[1])
    python_embed_main(algo)
