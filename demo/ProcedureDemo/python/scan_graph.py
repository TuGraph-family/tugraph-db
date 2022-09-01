#!/bin/python

import json

def Process(db, input):
    scan_edges = False
    try:
        param = json.loads(input)
        scan_edges = param['scan_edges']
    except:
        pass
    txn = db.CreateReadTxn()
    num_vertices = 0
    num_edges = 0
    vit = txn.GetVertexIterator()
    while vit.IsValid():
        num_vertices += 1
        if scan_edges:
            eit = vit.GetOutEdgeIterator()
            while eit.IsValid():
                num_edges += 1
                eit.Next()
        vit.Next()
    output = {"num_vertices":num_vertices}
    if scan_edges:
        output["num_edges"] = num_edges
    return (True, json.dumps(output))
