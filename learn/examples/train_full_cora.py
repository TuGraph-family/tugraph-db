import sys
sys.path.append("../../build/output/algo")
sys.path.append("../../build/output/")
import argparse
import time
import random
import torch
import torch.nn as nn
import torch.nn.functional as F
import dgl
import json
from threading import Thread, Lock
import queue
import os
from lgraph_db_python import *
import importlib

getdb = importlib.import_module("getdb")

feature_len = 1433
classes = 7

def remap(src, dst, nodes_idx):
    nodes_idx_map = {}
    for i in range(0, len(nodes_idx)):
        nodes_idx_map[nodes_idx[i]] = i
    for i in range(0, len(src)):
        src[i] = nodes_idx_map[src[i]]
    for i in range(0, len(dst)):
        dst[i] = nodes_idx_map[dst[i]]

# multi process training ref:
# https://docs.dgl.ai/en/0.8.x/tutorials/multi/2_node_classification.html#sphx-glr-tutorials-multi-2-node-classification-py

def get_graph(args):
    galaxy = PyGalaxy(args.db_path)
    galaxy.SetCurrentUser(args.username, args.password)
    db = galaxy.OpenGraph(args.graph_name, False)
    txn = db.CreateReadTxn()
    olapondb = PyOlapOnDB('Empty', db, txn)
    del txn

    NodeInfo = []
    EdgeInfo = []

    getdb.Process(db, olapondb, feature_len, NodeInfo, EdgeInfo)

    del db
    del galaxy

    src = EdgeInfo[0].astype('int64')
    dst = EdgeInfo[1].astype('int64')
    nodes_idx = NodeInfo[0].astype('int64')
    remap(src, dst, nodes_idx)
    features = NodeInfo[1].astype('float32')
    labels = NodeInfo[2].astype('int64')
    g = dgl.graph((src, dst))
    g.ndata['feat'] = torch.tensor(features)
    g.ndata['label'] = torch.tensor(labels)
    return g


class GCN(nn.Module):
    def __init__(self, in_size, hid_size, out_size):
        super().__init__()
        self.layers = nn.ModuleList()
        # two-layer GCN
        self.layers.append(dgl.nn.GraphConv(in_size, hid_size, activation=F.relu))
        self.layers.append(dgl.nn.GraphConv(hid_size, out_size))
        self.dropout = nn.Dropout(0.5)

    def forward(self, g, features):
        h = features
        for i, layer in enumerate(self.layers):
            if i != 0:
                h = self.dropout(h)
            h = layer(g, h)
        return h

def build_model():
    # ogbn-products feature length and classes
    in_size = feature_len
    out_size = classes
    model = GCN(in_size, 16, out_size)
    return model

loss_fcn = nn.CrossEntropyLoss()
def train(graph, model, model_save_path):
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-2, weight_decay=5e-4)
    model.train()
    s = time.time()
    load_time = time.time()
    graph = dgl.add_self_loop(graph)
    logits = model(graph, graph.ndata['feat'])
    loss = loss_fcn(logits, graph.ndata['label'])
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
    train_time = time.time()
    # print('load time', load_time - s, 'train_time', train_time - load_time)
    current_loss = float(loss)
    if model_save_path != "":
        if 'min_loss' not in train.__dict__:
            train.min_loss = current_loss
        elif current_loss < train.min_loss:
            train.min_loss = current_loss
            model_save_path = 'best_model.pth'
        torch.save(model.state_dict(), model_save_path)
    return current_loss


def run(proc_id, n_gpus, args, devices):
    dev_id = devices[proc_id]
    dist_init_method = 'tcp://{master_ip}:{master_port}'.format(master_ip='127.0.0.1', master_port='14455')
    if torch.cuda.device_count() < 1:
        device = torch.device('cpu')
        torch.distributed.init_process_group(
            backend='gloo', init_method=dist_init_method, world_size=len(devices), rank=proc_id)
    else:
        torch.cuda.set_device(dev_id)
        device = torch.device('cuda:' + str(dev_id))
        torch.distributed.init_process_group(
            backend='nccl', init_method=dist_init_method, world_size=len(devices), rank=proc_id)

    print('init process down')
    model = build_model()

    print('get_graph...')
    g = get_graph(args)
    # Dist model
    if device == torch.device('cpu'):
        model = torch.nn.parallel.DistributedDataParallel(model, device_ids=None, output_device=None)
    else:
        model = torch.nn.parallel.DistributedDataParallel(model, device_ids=[device], output_device=device)

    for epoch in range(20):
        print('training')
        model.train()
        total_loss = 0
        loss = train(g, model, args.model_save_path)
        print('loss', loss)
        if epoch == 19 and loss <= 0.9:
            print("The loss value is less than 0.9")
        sys.stdout.flush()


def main(args):
    n_gpus = 1
    devices = list(range(n_gpus))
    import torch.multiprocessing as mp
    torch.multiprocessing.freeze_support()
    mp.spawn(run, args=(n_gpus, args, devices), nprocs=n_gpus)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='random_sample')
    parser.add_argument("--db_path", type=str, default="./coradb",
                        help="database directory")
    parser.add_argument("--username", type=str, default="admin",
                        help="database username")
    parser.add_argument("--password", type=str, default="73@TuGraph",
                        help="database password")
    parser.add_argument("--graph_name", type=str, default="default",
                        help="import graph name")
    parser.add_argument("--model_save_path", type=str, default = "",
                        help="model path")
    args = parser.parse_args()
    print(args)
    main(args)
