import argparse
import time
import numpy as np
import torch
import torch.nn.functional as F
import dgl
from dgl.data import CoraGraphDataset, CiteseerGraphDataset, PubmedGraphDataset

import torch.nn as nn
from dgl.nn.pytorch import GraphConv

import json
from liblgraph_python_api import GraphDB, Galaxy


class GCN(nn.Module):
    def __init__(self,
                 g,
                 in_feats,
                 n_hidden,
                 n_classes,
                 n_layers,
                 activation,
                 dropout):
        super(GCN, self).__init__()
        self.g = g
        self.layers = nn.ModuleList()
        # input layer
        self.layers.append(GraphConv(in_feats, n_hidden, activation=activation))
        # hidden layers
        for i in range(n_layers - 1):
            self.layers.append(GraphConv(n_hidden, n_hidden, activation=activation))
        # output layer
        self.layers.append(GraphConv(n_hidden, n_classes))
        self.dropout = nn.Dropout(p=dropout)

    def forward(self, features):
        h = features
        for i, layer in enumerate(self.layers):
            if i != 0:
                h = self.dropout(h)
            h = layer(self.g, h)
        return h


def evaluate(model, features, labels, mask):
    model.eval()
    with torch.no_grad():
        logits = model(features)
        logits = logits[mask]
        labels = labels[mask]
        _, indices = torch.max(logits, dim=1)
        correct = torch.sum(indices == labels)
        return correct.item() * 1.0 / len(labels)


def main(args):
    vertices = []
    src = []
    dst = []
    features = []
    labels = []
    train_mask = []
    val_mask = []
    test_mask = []

    galaxy = Galaxy(args.db_path)
    galaxy.SetCurrentUser(args.username, args.password)
    graphDB = galaxy.OpenGraph(args.graph_name, False)
    txn = graphDB.CreateReadTxn()
    vit = txn.GetVertexIterator(0)

    while (True):
        nbr_list = vit.ListDstVids()
        vid = vit.GetId()
        vertices.append(vid)
        feat_str = vit.GetField("features")
        str_list = feat_str.split("_")
        double_list = []
        for i in range(0, len(str_list)):
            double_list.append(float(str_list[i]))
        # features.append(torch.tensor(double_list, dtype=torch.float32))
        features.append(double_list)
        labels.append(vit.GetField("label"))
        train_mask.append(vit.GetField("train_mask"))
        val_mask.append(vit.GetField("val_mask"))
        test_mask.append(vit.GetField("test_mask"))

        for nbr in nbr_list[0]:
            src.append(vid)
            dst.append(nbr)

        if (not vit.Next()):
            break
    txn.Commit()
    graphDB.Close()
    galaxy.Close()

    features = torch.tensor(features, dtype=torch.float32)
    labels = torch.tensor(labels, dtype=torch.int64)

    train_mask = torch.tensor(train_mask, dtype=torch.bool)
    val_mask = torch.tensor(val_mask, dtype=torch.bool)
    test_mask = torch.tensor(test_mask, dtype=torch.bool)

    src, dst = torch.tensor(src, dtype=torch.int32), torch.tensor(dst, dtype=torch.int32)
    g = dgl.graph((src, dst))
    g.ndata['feat'] = features
    g.ndata['label'] = labels
    g.ndata['train_mask'] = train_mask
    g.ndata['val_mask'] = val_mask
    g.ndata['test_mask'] = test_mask

    if args.gpu < 0:
        cuda = False
    else:
        cuda = True
        g = g.int().to(args.gpu)
    in_feats = features.shape[1]

    # n_classes = data.num_labels
    label_set = set()
    for ele_torch in labels:
        ele = ele_torch.item()
        if ele not in label_set:
            label_set.add(ele)
    n_classes = len(label_set)

    # n_edges = data.graph.number_of_edges()
    n_edges = len(src)
    print("n_edges = ", n_edges)

    print("""----Data statistics------'
      #Edges %d
      #Classes %d
      #Train samples %d
      #Val samples %d
      #Test samples %d""" %
          (n_edges, n_classes,
           train_mask.int().sum().item(),
           val_mask.int().sum().item(),
           test_mask.int().sum().item()))

    # add self loop
    if args.self_loop:
        g = dgl.remove_self_loop(g)
        g = dgl.add_self_loop(g)
    n_edges = g.number_of_edges()

    # normalization
    degs = g.in_degrees().float()
    norm = torch.pow(degs, -0.5)
    norm[torch.isinf(norm)] = 0
    if cuda:
        norm = norm.cuda()
    g.ndata['norm'] = norm.unsqueeze(1)

    # create GCN model
    model = GCN(g,
                in_feats,
                args.n_hidden,
                n_classes,
                args.n_layers,
                F.relu,
                args.dropout)

    if cuda:
        model.cuda()
    loss_fcn = torch.nn.CrossEntropyLoss()

    # use optimizer
    optimizer = torch.optim.Adam(model.parameters(),
                                 lr=args.lr,
                                 weight_decay=args.weight_decay)

    # initialize graph
    dur = []
    for epoch in range(args.n_epochs):
        model.train()
        if epoch >= 3:
            t0 = time.time()
        # forward
        logits = model(features)
        loss = loss_fcn(logits[train_mask], labels[train_mask])

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()

        if epoch >= 3:
            dur.append(time.time() - t0)

        acc = evaluate(model, features, labels, val_mask)
        print("Epoch {:05d} | Time(s) {:.4f} | Loss {:.4f} | Accuracy {:.4f} | "
              "ETputs(KTEPS) {:.2f}".format(epoch, np.mean(dur), loss.item(),
                                            acc, n_edges / np.mean(dur) / 1000))

    print()
    acc = evaluate(model, features, labels, test_mask)
    print("Test accuracy {:.2%}".format(acc))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='GCN')

    parser.add_argument("--db_path", type=str, default="./db_cora",
                        help="database directory")
    parser.add_argument("--username", type=str, default="admin",
                        help="database username")
    parser.add_argument("--password", type=str,
                        help="database password")
    parser.add_argument("--graph_name", type=str, default="default",
                        help="import graph name")

    parser.add_argument("--dropout", type=float, default=0.5,
                        help="dropout probability")
    parser.add_argument("--gpu", type=int, default=-1,
                        help="gpu")
    parser.add_argument("--lr", type=float, default=1e-2,
                        help="learning rate")
    parser.add_argument("--n-epochs", type=int, default=200,
                        help="number of training epochs")
    parser.add_argument("--n-hidden", type=int, default=16,
                        help="number of hidden gcn units")
    parser.add_argument("--n-layers", type=int, default=1,
                        help="number of hidden gcn layers")
    parser.add_argument("--weight-decay", type=float, default=5e-4,
                        help="Weight for L2 loss")
    parser.add_argument("--self-loop", action='store_true',
                        help="graph self-loop (default=False)")
    parser.set_defaults(self_loop=False)
    args = parser.parse_args()
    print(args)

    main(args)
