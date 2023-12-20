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
import torch as th
import torch.nn as nn
from dgl import AddReverse, Compose, ToSimple
from dgl.nn import HeteroEmbedding
import dgl.nn as dglnn
from tqdm import tqdm
import itertools
import psutil
from ogb.nodeproppred import DglNodePropPredDataset, Evaluator

v_t = dgl.__version__

getdb = importlib.import_module("getdb")

feature_len = 128
classes = 7

def remap(src, dst, nodes_idx):
    nodes_idx_map = {}
    for i in range(0, len(nodes_idx)):
        nodes_idx_map[nodes_idx[i]] = i
    for i in range(0, len(src)):
        src[i] = nodes_idx_map[src[i]]
    for i in range(0, len(dst)):
        dst[i] = nodes_idx_map[dst[i]]

def remap_with_types(src, dst, nodes_idx, node_types):
    type_index_map = {}
    for i in range(0, len(nodes_idx)):
        thetype = node_types[i]
        if thetype not in type_index_map:
            type_index_map[thetype] = 0
            nodes_idx[i] = 0
        else:
            type_index_map[thetype] += 1
            nodes_idx[i] = type_index_map[thetype]
    for i in range(0, len(src)):
        src[i] = nodes_idx[src[i]]
    for i in range(0, len(dst)):
        dst[i] = nodes_idx[dst[i]]

# multi process training ref:
# https://docs.dgl.ai/en/0.8.x/tutorials/multi/2_node_classification.html#sphx-glr-tutorials-multi-2-node-classification-py

def prepare_data(args, device):
    meta_edge_types = ["author___affiliated_with___institution", "paper___cites___paper", "paper___has_topic___field_of_study", "author___writes___paper"]
    # meta_edge_types = ["paper___cites___paper", "author___writes___paper"]
    graph_data = {}
    galaxy = PyGalaxy(args.db_path)
    galaxy.SetCurrentUser(args.username, args.password)
    db = galaxy.OpenGraph(args.graph_name, False)

    features = []
    labels = []
    for edge_type in meta_edge_types:
        print("start edge_type", edge_type)
        txn = db.CreateReadTxn()
        edge_type_split = edge_type.split("___")
        node_types = [edge_type_split[0], edge_type_split[2]]
        action = edge_type_split[1]
        # olapondb = PyOlapOnDB('Empty', db, txn, node_types, [edge_type])
        edge_tuple = (edge_type_split[0], edge_type, edge_type_split[2])
        print(edge_tuple)
        olapondb = PyOlapOnDB('Empty', db, txn, [edge_tuple])
        del txn
        NodeInfo = []
        EdgeInfo = []
        getdb.Process(db, olapondb, feature_len, NodeInfo, EdgeInfo)
        src = EdgeInfo[0].astype('int64')
        dst = EdgeInfo[1].astype('int64')
        nodes_idx = NodeInfo[0].astype('int64')
        remap_with_types(src, dst, nodes_idx, NodeInfo[3])
        if edge_type == 'paper___cites___paper':
            features = NodeInfo[1].astype('float32')
            labels = NodeInfo[2].astype('int64')
        graph_data[(node_types[0], action, node_types[1])] = (src, dst)

    # delete as soon as possible
    del db
    del galaxy

    g = dgl.heterograph(graph_data)
    g.nodes['paper'].data['feat'] = torch.tensor(features)
    g.nodes['paper'].data['label'] = torch.tensor(labels)
    print('feat', g.nodes['paper'].data['feat'])
    print('label', g.nodes['paper'].data['feat'])
    print('g:\n', g)
    num_classes = 349

    split_idx = {}
    split_idx['train'] = {}
    split_idx['valid'] = {}
    split_idx['test'] = {}
    num_nodes = len(labels)
    split_idx['train']['paper'] = torch.arange(0, num_nodes)
    valid_nodes = int(0.6*num_nodes)
    split_idx['valid']['paper'] = torch.arange(0, valid_nodes)
    split_idx['test']['paper'] = torch.arange(valid_nodes, num_nodes)
    print(split_idx)

    logger = Logger(args.runs)
    sampler = dgl.dataloading.MultiLayerNeighborSampler([25, 20])
    train_loader = dgl.dataloading.DataLoader(
        g,
        split_idx["train"],
        sampler,
        batch_size=1024,
        shuffle=True,
        num_workers=args.num_workers,
        device=device,
    )

    return g, g.nodes['paper'].data['label'], num_classes, split_idx, logger, train_loader

def extract_embed(node_embed, input_nodes):
    emb = node_embed(
        {ntype: input_nodes[ntype] for ntype in input_nodes if ntype != "paper"}
    )
    return emb


def rel_graph_embed(graph, embed_size):
    node_num = {}
    for ntype in graph.ntypes:
        if ntype == "paper":
            continue
        node_num[ntype] = graph.num_nodes(ntype)
    embeds = HeteroEmbedding(node_num, embed_size)
    return embeds

class RelGraphConvLayer(nn.Module):
    def __init__(
        self, in_feat, out_feat, ntypes, rel_names, activation=None, dropout=0.0
    ):
        super(RelGraphConvLayer, self).__init__()
        self.in_feat = in_feat
        self.out_feat = out_feat
        self.ntypes = ntypes
        self.rel_names = rel_names
        self.activation = activation

        self.conv = dglnn.HeteroGraphConv(
            {
                rel: dglnn.GraphConv(
                    in_feat, out_feat, norm="right", weight=False, bias=False
                )
                for rel in rel_names
            }
        )

        self.weight = nn.ModuleDict(
            {
                rel_name: nn.Linear(in_feat, out_feat, bias=False)
                for rel_name in self.rel_names
            }
        )

        # weight for self loop
        self.loop_weights = nn.ModuleDict(
            {
                ntype: nn.Linear(in_feat, out_feat, bias=True)
                for ntype in self.ntypes
            }
        )

        self.dropout = nn.Dropout(dropout)
        self.reset_parameters()

    def reset_parameters(self):
        for layer in self.weight.values():
            layer.reset_parameters()

        for layer in self.loop_weights.values():
            layer.reset_parameters()

    def forward(self, g, inputs):
        """
        Parameters
        ----------
        g : DGLGraph
            Input graph.
        inputs : dict[str, torch.Tensor]
            Node feature for each node type.

        Returns
        -------
        dict[str, torch.Tensor]
            New node features for each node type.
        """
        g = g.local_var()
        wdict = {
            rel_name: {"weight": self.weight[rel_name].weight.T}
            for rel_name in self.rel_names
        }

        inputs_dst = {
            k: v[: g.number_of_dst_nodes(k)] for k, v in inputs.items()
        }

        hs = self.conv(g, inputs, mod_kwargs=wdict)
        def _apply(ntype, h):
            h = h + self.loop_weights[ntype](inputs_dst[ntype])
            if self.activation:
                h = self.activation(h)
            return self.dropout(h)

        return {ntype: _apply(ntype, h) for ntype, h in hs.items()}



class EntityClassify(nn.Module):
    def __init__(self, g, in_dim, out_dim):
        super(EntityClassify, self).__init__()
        self.in_dim = in_dim
        self.h_dim = 64
        self.out_dim = out_dim
        self.rel_names = list(set(g.etypes))
        self.rel_names.sort()
        self.dropout = 0.5

        self.layers = nn.ModuleList()
        # i2h
        self.layers.append(
            RelGraphConvLayer(
                self.in_dim,
                self.h_dim,
                g.ntypes,
                self.rel_names,
                activation=F.relu,
                dropout=self.dropout,
            )
        )

        # h2o
        self.layers.append(
            RelGraphConvLayer(
                self.h_dim,
                self.out_dim,
                g.ntypes,
                self.rel_names,
                activation=None,
            )
        )

    def reset_parameters(self):
        for layer in self.layers:
            layer.reset_parameters()

    def forward(self, h, blocks):
        for layer, block in zip(self.layers, blocks):
            h = layer(block, h)
        return h


class Logger(object):
    r"""
    This class was taken directly from the PyG implementation and can be found
    here: https://github.com/snap-stanford/ogb/blob/master/examples/nodeproppred/mag/logger.py

    This was done to ensure that performance was measured in precisely the same way
    """

    def __init__(self, runs):
        self.results = [[] for _ in range(runs)]

    def add_result(self, run, result):
        assert len(result) == 3
        assert run >= 0 and run < len(self.results)
        self.results[run].append(result)

    def print_statistics(self, run=None):
        if run is not None:
            result = 100 * th.tensor(self.results[run])
            argmax = result[:, 1].argmax().item()
            print(f"Run {run + 1:02d}:")
            print(f"Highest Train: {result[:, 0].max():.2f}")
            print(f"Highest Valid: {result[:, 1].max():.2f}")
            print(f"  Final Train: {result[argmax, 0]:.2f}")
            print(f"   Final Test: {result[argmax, 2]:.2f}")
        else:
            result = 100 * th.tensor(self.results)

            best_results = []
            for r in result:
                train1 = r[:, 0].max().item()
                valid = r[:, 1].max().item()
                train2 = r[r[:, 1].argmax(), 0].item()
                test = r[r[:, 1].argmax(), 2].item()
                best_results.append((train1, valid, train2, test))

            best_result = th.tensor(best_results)

            print(f"All runs:")
            r = best_result[:, 0]
            print(f"Highest Train: {r.mean():.2f} ± {r.std():.2f}")
            r = best_result[:, 1]
            print(f"Highest Valid: {r.mean():.2f} ± {r.std():.2f}")
            r = best_result[:, 2]
            print(f"  Final Train: {r.mean():.2f} ± {r.std():.2f}")
            r = best_result[:, 3]
            print(f"   Final Test: {r.mean():.2f} ± {r.std():.2f}")





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

def train(
    g,
    model,
    node_embed,
    optimizer,
    train_loader,
    split_idx,
    labels,
    logger,
    device,
    run,
):
    print("start training...")
    category = "paper"

    for epoch in range(5):
        num_train = split_idx["train"][category].shape[0]
        pbar = tqdm(total=num_train)
        pbar.set_description(f"Epoch {epoch:02d}")
        model.train()

        total_loss = 0

        for input_nodes, seeds, blocks in train_loader:
            blocks = [blk.to(device) for blk in blocks]
            seeds = seeds[
                category
            ]  # we only predict the nodes with type "category"
            batch_size = seeds.shape[0]
            input_nodes_indexes = input_nodes["paper"].to(g.device)
            seeds = seeds.to(labels.device)

            emb = extract_embed(node_embed, input_nodes)
            # Add the batch's raw "paper" features
            emb.update({"paper": g.ndata["feat"]["paper"][input_nodes_indexes]})

            emb = {k: e.to(device) for k, e in emb.items()}
            lbl = labels[seeds].to(device)

            optimizer.zero_grad()
            logits = model(emb, blocks)[category]

            y_hat = logits.log_softmax(dim=-1)
            loss = F.nll_loss(y_hat, lbl)
            loss.backward()
            optimizer.step()

            total_loss += loss.item() * batch_size
            pbar.update(batch_size)

        pbar.close()
        loss = total_loss / num_train

        result = test(g, model, node_embed, labels, device, split_idx)
        logger.add_result(run, result)
        train_acc, valid_acc, test_acc = result
        print(
            f"Run: {run + 1:02d}, "
            f"Epoch: {epoch +1 :02d}, "
            f"Loss: {loss:.4f}, "
            f"Train: {100 * train_acc:.2f}%, "
            f"Valid: {100 * valid_acc:.2f}%, "
            f"Test: {100 * test_acc:.2f}%"
        )

    return logger


@th.no_grad()
def test(g, model, node_embed, y_true, device, split_idx):
    # test_batch_size = 16384
    test_batch_size = 16
    model.eval()
    category = "paper"
    evaluator = Evaluator(name="ogbn-mag")

    # 2 GNN layers
    sampler = dgl.dataloading.MultiLayerFullNeighborSampler(2)
    loader = dgl.dataloading.DataLoader(
        g,
        {"paper": th.arange(g.num_nodes("paper"))},
        sampler,
        batch_size=test_batch_size,
        shuffle=False,
        num_workers=0,
        device=device,
    )

    pbar = tqdm(total=y_true.size(0))
    pbar.set_description(f"Inference")

    y_hats = list()

    for input_nodes, seeds, blocks in loader:
        blocks = [blk.to(device) for blk in blocks]
        seeds = seeds[
            category
        ]  # we only predict the nodes with type "category"
        batch_size = seeds.shape[0]
        input_nodes_indexes = input_nodes["paper"].to(g.device)

        emb = extract_embed(node_embed, input_nodes)
        # Get the batch's raw "paper" features
        emb.update({"paper": g.ndata["feat"]["paper"][input_nodes_indexes]})
        emb = {k: e.to(device) for k, e in emb.items()}

        logits = model(emb, blocks)[category]
        y_hat = logits.log_softmax(dim=-1).argmax(dim=1, keepdims=True)
        y_hats.append(y_hat.cpu())

        pbar.update(batch_size)

    pbar.close()

    y_pred = th.cat(y_hats, dim=0)
    y_true = th.unsqueeze(y_true, 1)

    train_acc = evaluator.eval(
        {
            "y_true": y_true[split_idx["train"]["paper"]],
            "y_pred": y_pred[split_idx["train"]["paper"]],
        }
    )["acc"]
    valid_acc = evaluator.eval(
        {
            "y_true": y_true[split_idx["valid"]["paper"]],
            "y_pred": y_pred[split_idx["valid"]["paper"]],
        }
    )["acc"]
    test_acc = evaluator.eval(
        {
            "y_true": y_true[split_idx["test"]["paper"]],
            "y_pred": y_pred[split_idx["test"]["paper"]],
        }
    )["acc"]

    return train_acc, valid_acc, test_acc


def is_support_affinity(v_t):
    # dgl supports enable_cpu_affinity since 0.9.1
    return v_t >= "0.9.1"

def main(args):
    # device = f"cuda:0" if th.cuda.is_available() else "cpu"
    device = "cpu"

    g, labels, num_classes, split_idx, logger, train_loader = prepare_data(
        args, device
    )

    embed_layer = rel_graph_embed(g, 128).to(device)
    model = EntityClassify(g, 128, num_classes).to(device)

    print(
        f"Number of embedding parameters: {sum(p.numel() for p in embed_layer.parameters())}"
    )
    print(
        f"Number of model parameters: {sum(p.numel() for p in model.parameters())}"
    )

    for run in range(args.runs):
        try:
            embed_layer.reset_parameters()
            model.reset_parameters()
        except:
            # old pytorch version doesn't support reset_parameters() API
            pass

        # optimizer
        all_params = itertools.chain(
            model.parameters(), embed_layer.parameters()
        )
        optimizer = th.optim.Adam(all_params, lr=0.01)

        if (
            args.num_workers != 0
            and device == "cpu"
            and is_support_affinity(v_t)
        ):
            expected_max = int(psutil.cpu_count(logical=False))
            if args.num_workers >= expected_max:
                print(
                    f"[ERROR] You specified num_workers are larger than physical cores, please set any number less than {expected_max}",
                    file=sys.stderr,
                )
            with train_loader.enable_cpu_affinity():
                logger = train(
                    g,
                    model,
                    embed_layer,
                    optimizer,
                    train_loader,
                    split_idx,
                    labels,
                    logger,
                    device,
                    run,
                )
        else:
            logger = train(
                g,
                model,
                embed_layer,
                optimizer,
                train_loader,
                split_idx,
                labels,
                logger,
                device,
                run,
            )
        logger.print_statistics(run)

    print("Final performance: ")
    logger.print_statistics()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='random_sample')
    parser.add_argument("--db_path", type=str, default="./magdb",
                        help="database directory")
    parser.add_argument("--username", type=str, default="admin",
                        help="database username")
    parser.add_argument("--password", type=str, default="73@TuGraph",
                        help="database password")
    parser.add_argument("--graph_name", type=str, default="default",
                        help="import graph name")
    parser.add_argument("--model_save_path", type=str, default = "",
                        help="model path")
    parser.add_argument("--runs", type=int, default=2)
    parser.add_argument("--num_workers", type=int, default=0)
    args = parser.parse_args()
    print(args)
    main(args)

