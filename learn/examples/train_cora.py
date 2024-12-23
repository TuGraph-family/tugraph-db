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

feature_len = 1433
classes = 7

batch_size = 2
train_nids = 2708

rw_len = 2

p = 10
q = 1
walk_length = 10
num_walks = 10


def construct_graph():
    src_ids = [0, 2, 3, 4]
    dst_ids = [1, 1, 2, 3]
    g = dgl.graph((src_ids, dst_ids))
    return g

# multi process training ref:
# https://docs.dgl.ai/en/0.8.x/tutorials/multi/2_node_classification.html#sphx-glr-tutorials-multi-2-node-classification-py

def remap(src, dst, nodes_idx):
    nodes_idx_map = {}
    for i in range(0, len(nodes_idx)):
        nodes_idx_map[nodes_idx[i]] = i
    for i in range(0, len(src)):
        src[i] = nodes_idx_map[src[i]]
    for i in range(0, len(dst)):
        dst[i] = nodes_idx_map[dst[i]]


class TugraphSample(object):
    def __init__(self, args=None):
        super(TugraphSample, self).__init__()
        self.args = args
        self.algo = importlib.import_module(args.method)


    def process(self, db, olapondb, seed_nodes, NodeInfo, EdgeInfo):
        args = self.args
        if args.method == "neighbors_sampling":
            self.algo.Process(db, olapondb, feature_len,
                    seed_nodes, args.neighbor_sample_num,
                    NodeInfo, EdgeInfo)
        elif args.method == "random_walk":
            self.algo.Process(db, olapondb, feature_len,
                    seed_nodes, rw_len, NodeInfo, EdgeInfo)
        elif args.method == "edge_sampling":
            self.algo.Process(db, olapondb, feature_len, args.sample_rate,
                    NodeInfo, EdgeInfo)
        elif args.method == "negative_sampling":
            self.algo.Process(db, olapondb, feature_len,
                    len(seed_nodes), NodeInfo, EdgeInfo)
        elif args.method == "node2vec_sampling":
            self.algo.Process(db, olapondb, feature_len, p, q, walk_length, num_walks,
                    seed_nodes, NodeInfo, EdgeInfo)


    def sample(self, g, seed_nodes):
        args = self.args
        galaxy = PyGalaxy(args.db_path)
        galaxy.SetCurrentUser(args.username, args.password)
        db = galaxy.OpenGraph(args.graph_name, False)
        txn = db.CreateReadTxn()
        olapondb = PyOlapOnDB('Empty', db, txn)
        del txn

        seed_nodes = seed_nodes.tolist()
        NodeInfo = []
        EdgeInfo = []
        self.process(db, olapondb, seed_nodes, NodeInfo, EdgeInfo)
        del db
        del galaxy

        remap(EdgeInfo[0], EdgeInfo[1], NodeInfo[0])
        g = dgl.graph((EdgeInfo[0], EdgeInfo[1]))
        g.ndata['feat'] = torch.tensor(NodeInfo[1])
        g.ndata['label'] = torch.tensor(NodeInfo[2])
        return g

def test_fork(args):
    src = []
    dst = []
    features = []
    labels = []
    output_dir = args.output_dir

    pid = os.fork()
    if pid > 0:
        start_time = time.time()
        galaxy = PyGalaxy(args.db_path)
        galaxy.SetCurrentUser(args.username, args.password)
        db = galaxy.OpenGraph(args.graph_name, False)
        sample_num = python_plugin.Process(db, args.sample_rate, src, dst, features, labels)
        del db
        del galaxy
        end_time = time.time()
    else:
        start_time = time.time()
        galaxy = PyGalaxy(args.db_path)
        galaxy.SetCurrentUser(args.username, args.password)
        db = galaxy.OpenGraph(args.graph_name, False)
        sample_num = python_plugin.Process(db, args.sample_rate, src, dst, features, labels)
        del db
        del galaxy
        end_time = time.time()

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
    # in_size = 100
    # out_size = 47
    # cora
    in_size = feature_len
    out_size = classes
    model = GCN(in_size, 16, out_size)
    return model

loss_fcn = nn.CrossEntropyLoss()
def train(dataloader, model):
    optimizer = torch.optim.Adam(model.parameters(), lr=1e-2, weight_decay=5e-4)
    model.train()
    s = time.time()
    for graph in dataloader:
        load_time = time.time()
        graph = dgl.add_self_loop(graph)
        logits = model(graph, graph.ndata['feat'])
        loss = loss_fcn(logits, graph.ndata['label'])
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        train_time = time.time()
        # print('load time', load_time - s, 'train_time', train_time - load_time)
        print('loss:  ', float(loss))
        s = time.time()
    return float(loss)

def singleprocess(args):
    batch_size = 5
    count = 2708
    sampler = TugraphSample(args)
    fake_g = construct_graph() # just make dgl happy
    dataloader = dgl.dataloading.DataLoader(fake_g,
        torch.arange(count),
        sampler,
        batch_size=batch_size,
        num_workers=0,
        )
    start = time.time()
    for i, subg in enumerate(dataloader):
        print('subg, ', subg)
        if i > 1:
            break
    print('dataloader time: ', time.time() - start)

    model = build_model()
    for epoch in range(2):
        model.train()
        total_loss = 0
        loss = train(dataloader, model)
        print('epoch:', epoch, 'loss:', loss)
        if epoch == 1 and loss <= 0.9:
            print("The loss value is less than 0.9")
        sys.stdout.flush()

def run(proc_id, n_gpus, args, devices):
    dev_id = devices[proc_id]
    dist_init_method = 'tcp://{master_ip}:{master_port}'.format(master_ip='127.0.0.1', master_port='12346')
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
    sampler = TugraphSample(args)
    fake_g = construct_graph() # just make dgl happy
    dataloader = dgl.dataloading.DataLoader(fake_g,
        torch.arange(train_nids),
        sampler,
        batch_size=batch_size,
        device=device,
        use_ddp=True,
        num_workers=0,
        drop_last=False,
        )
    sys.stdout.flush()
    start = time.time()
    for i, subg in enumerate(dataloader):
        sys.stdout.flush()
        if i > 2:
            break
    sys.stdout.flush()
    print('dataloader time: ', time.time() - start)

    model = build_model()

    # Dist model
    if device == torch.device('cpu'):
        model = torch.nn.parallel.DistributedDataParallel(model, device_ids=None, output_device=None)
    else:
        model = torch.nn.parallel.DistributedDataParallel(model, device_ids=[device], output_device=device)

    for epoch in range(100):
        model.train()
        total_loss = 0
        loss = train(dataloader, model)
        print('epoch:', epoch, 'loss:', loss)
        sys.stdout.flush()


def main(args):
    n_gpus = 2
    devices = list(range(n_gpus))
    import torch.multiprocessing as mp
    torch.multiprocessing.freeze_support()
    # mp.spawn(run, args=(n_gpus, args, devices), nprocs=n_gpus)
    singleprocess(args)


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
    parser.add_argument("--sample_rate", type=float, default="0.01",
                        help="the rate of sample edge")
    parser.add_argument("--output_dir", type=str, default="./sample_info.txt",
                        help="the path to store vertex info")
    parser.add_argument('--method', type=str, default='neighbors_sampling',
                        help='sample method:\
                        neighbors_sampling, edge_sampling, random_walk, negative_sampling, node2vec_sampling')
    parser.add_argument('--neighbor_sample_num', type=int, default=20,
                        help='neighbor sampling number.')
    parser.add_argument('--randomwalk_length', type=int, default=20,
                        help='randomwalk length.')

    args = parser.parse_args()
    print(args)
    main(args)
