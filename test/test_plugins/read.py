from liblgraph_python_api import *

def Process(db, input):
    n = 0
    try:
        n = int(input)
    except:
        pass
    if n == 0:
        n = 1000000
    txn = db.CreateReadTxn()
    it = txn.GetVertexIterator()
    nv = 0
    while it.IsValid() and nv < n:
        nv = nv + 1
        it.Next()
    return (True, str(nv))