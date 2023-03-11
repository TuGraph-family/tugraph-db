# import networkx as nx
import torch
import dgl
import dgl.function as fn

import json
import typing
from liblgraph_python_api import GraphDB, Galaxy

import argparse

DAMP = 0.85


def compute_pagerank(g, iterations):
    num_vertices = g.number_of_nodes()
    g.ndata['pv'] = torch.ones(num_vertices) / num_vertices
    degrees = g.out_degrees(g.nodes()).type(torch.float32)
    for iter in range(iterations):
        g.ndata['pv'] = g.ndata['pv'] / degrees
        g.update_all(message_func=fn.copy_src(src='pv', out='m'),
                     reduce_func=fn.sum(msg='m', out='pv'))
        g.ndata['pv'] = (1 - DAMP) / num_vertices + DAMP * g.ndata['pv']
    return g.ndata['pv']


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='pagerank')

    parser.add_argument("--db_path", type=str, default="./db_cora",
                        help="database directory")
    parser.add_argument("--username", type=str, default="admin",
                        help="database username")
    parser.add_argument("--password", type=str,
                        help="database password")
    parser.add_argument("--graph_name", type=str, default="default",
                        help="import graph name")
    parser.add_argument("--iterations", type=int, default=10,
                        help="iterations to run pagerank")
    args = parser.parse_args()

    galaxy = Galaxy(args.db_path)
    galaxy.SetCurrentUser(args.username, args.password)
    graphDB = galaxy.OpenGraph(args.graph_name, False)
    txn = graphDB.CreateReadTxn()

    src_list = []
    dst_list = []
    vit = txn.GetVertexIterator(0)
    while (True):
        nbr_list = vit.ListDstVids()
        src = vit.GetId()
        for nbr in nbr_list[0]:
            src_list.append(src)
            dst_list.append(nbr)
        if (not vit.Next()):
            break

    u, v = torch.tensor(src_list), torch.tensor(dst_list)
    g = dgl.graph((u, v))

    pv = compute_pagerank(g, args.iterations)
    print(pv)
    txn.Commit()
    graphDB.Close()
    galaxy.Close()
