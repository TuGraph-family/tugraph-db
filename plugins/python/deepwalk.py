import torch
import argparse
import dgl
import torch.multiprocessing as mp
from torch.utils.data import DataLoader
import os
import random
import time
import numpy as np

# from ogb.linkproppred import DglLinkPropPredDataset


import torch.nn as nn
import torch.nn.functional as F
from torch.nn import init
from torch.multiprocessing import Queue

import scipy.sparse as sp
import pickle
from dgl.data.utils import download, _get_dgl_url, get_download_dir, extract_archive

import json
from liblgraph_python_api import GraphDB, Galaxy


def shuffle_walks(walks):
    seeds = torch.randperm(walks.size()[0])
    return walks[seeds]


def sum_up_params(model):
    """ Count the model parameters """
    n = []
    n.append(model.u_embeddings.weight.cpu().data.numel() * 2)
    n.append(model.lookup_table.cpu().numel())
    n.append(model.index_emb_posu.cpu().numel() * 2)
    n.append(model.grad_u.cpu().numel() * 2)

    try:
        n.append(model.index_emb_negu.cpu().numel() * 2)
    except:
        pass
    try:
        n.append(model.state_sum_u.cpu().numel() * 2)
    except:
        pass
    try:
        n.append(model.grad_avg.cpu().numel())
    except:
        pass
    try:
        n.append(model.context_weight.cpu().numel())
    except:
        pass

    print("#params " + str(sum(n)))
    exit()


def ReadTxtNet(file_path="", undirected=True):
    """ Read the txt network file.
    Notations: The network is unweighted.

    Parameters
    ----------
    file_path str : path of network file
    undirected bool : whether the edges are undirected

    Return
    ------
    net dict : a dict recording the connections in the graph
    node2id dict : a dict mapping the nodes to their embedding indices
    id2node dict : a dict mapping nodes embedding indices to the nodes
    """
    if file_path == 'youtube' or file_path == 'blog':
        name = file_path
        dir = get_download_dir()
        zip_file_path = '{}/{}.zip'.format(dir, name)
        download(_get_dgl_url(os.path.join('dataset/DeepWalk/', '{}.zip'.format(file_path))), path=zip_file_path)
        extract_archive(zip_file_path,
                        '{}/{}'.format(dir, name))
        file_path = "{}/{}/{}-net.txt".format(dir, name, name)

    node2id = {}
    id2node = {}
    cid = 0

    src = []
    dst = []
    weight = []
    net = {}
    with open(file_path, "r") as f:
        for line in f.readlines():
            tup = list(map(int, line.strip().split(" ")))
            assert len(tup) in [2, 3], "The format of network file is unrecognizable."
            if len(tup) == 3:
                n1, n2, w = tup
            elif len(tup) == 2:
                n1, n2 = tup
                w = 1
            if n1 not in node2id:
                node2id[n1] = cid
                id2node[cid] = n1
                cid += 1
            if n2 not in node2id:
                node2id[n2] = cid
                id2node[cid] = n2
                cid += 1

            n1 = node2id[n1]
            n2 = node2id[n2]
            if n1 not in net:
                net[n1] = {n2: w}
                src.append(n1)
                dst.append(n2)
                weight.append(w)
            elif n2 not in net[n1]:
                net[n1][n2] = w
                src.append(n1)
                dst.append(n2)
                weight.append(w)

            if undirected:
                if n2 not in net:
                    net[n2] = {n1: w}
                    src.append(n2)
                    dst.append(n1)
                    weight.append(w)
                elif n1 not in net[n2]:
                    net[n2][n1] = w
                    src.append(n2)
                    dst.append(n1)
                    weight.append(w)

    print("node num: %d" % len(net))
    print("edge num: %d" % len(src))
    assert max(net.keys()) == len(net) - 1, "error reading net, quit"

    sm = sp.coo_matrix(
        (np.array(weight), (src, dst)),
        dtype=np.float32)

    return net, node2id, id2node, sm


def ReadTugrpahNet(args):
    node2id = {}
    id2node = {}
    cid = 0

    src = []
    dst = []
    weight = []
    net = {}

    galaxy = Galaxy(args.data_file)
    galaxy.SetCurrentUser(args.username, args.password)
    graphDB = galaxy.OpenGraph(args.graph_name, False)
    txn = graphDB.CreateReadTxn()
    vit = txn.GetVertexIterator(0)

    while (True):
        nbr_list = vit.ListDstVids()
        vid = vit.GetId()
        for nbr in nbr_list[0]:
            n1 = vid
            n2 = nbr
            w = 1
            if n1 not in node2id:
                node2id[n1] = cid
                id2node[cid] = n1
                cid += 1
            if n2 not in node2id:
                node2id[n2] = cid
                id2node[cid] = n2
                cid += 1

            n1 = node2id[n1]
            n2 = node2id[n2]
            if n1 not in net:
                net[n1] = {n2: w}
                src.append(n1)
                dst.append(n2)
                weight.append(w)
            elif n2 not in net[n1]:
                net[n1][n2] = w
                src.append(n1)
                dst.append(n2)
                weight.append(w)

            if args.make_undirected:
                if n2 not in net:
                    net[n2] = {n1: w}
                    src.append(n2)
                    dst.append(n1)
                    weight.append(w)
                elif n1 not in net[n2]:
                    net[n2][n1] = w
                    src.append(n2)
                    dst.append(n1)
                    weight.append(w)
        if (not vit.Next()):
            break

    txn.Commit()
    graphDB.Close()
    galaxy.Close()

    print("node num: %d" % len(net))
    print("edge num: %d" % len(src))
    assert max(net.keys()) == len(net) - 1, "error reading net, quit"

    print("before coo_matrix")
    sm = sp.coo_matrix(
        (np.array(weight), (src, dst)),
        dtype=np.float32)

    print("after read from tugraph")

    return net, node2id, id2node, sm


class DeepwalkSampler(object):
    def __init__(self, G, seeds, walk_length):
        """ random walk sampler

        Parameter
        ---------
        G dgl.Graph : the input graph
        seeds torch.LongTensor : starting nodes
        walk_length int : walk length
        """
        self.G = G
        self.seeds = seeds
        self.walk_length = walk_length

    def sample(self, seeds):
        walks = dgl.sampling.random_walk(self.G, seeds, length=self.walk_length - 1)[0]
        return walks


"""
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', type=str,
                        choices=['ogbl-collab', 'ogbl-ddi', 'ogbl-ppa', 'ogbl-citation'],
                        default='ogbl-collab',
                        help="name of datasets by ogb")
    args = parser.parse_args()

    print("loading graph... it might take some time")
    name = args.name
    g = load_from_ogbl_with_name(name=name)

    try:
        w = g.edata['edge_weight']
        weighted = True
    except:
        weighted = False


    edge_num = g.edges()[0].shape[0]
    src = list(g.edges()[0])
    tgt = list(g.edges()[1])
    if weighted:
        weight = list(g.edata['edge_weight'])

    print("writing...")
    start_time = time.time()
    with open(name + "-net.txt", "w") as f:
        for i in range(edge_num):
            if weighted:
                f.write(str(src[i].item()) + " " \
                        +str(tgt[i].item()) + " " \
                        +str(weight[i].item()) + "\n")
            else:
                f.write(str(src[i].item()) + " " \
                        +str(tgt[i].item()) + " " \
                        +"1\n")
    print("writing used time: %d s" % int(time.time() - start_time))
"""


def net2graph(net_sm):
    """ Transform the network to DGL graph

    Parameters
    __________
    net_sm matrix : sparse matrix of edgelist

    Return
    ------
    G DGLGraph : graph by DGL
    """
    start = time.time()
    G = dgl.DGLGraph(net_sm)
    end = time.time()
    t = end - start
    print("Building DGLGraph in %.2fs" % t)
    return G


def make_undirected(G):
    # G.readonly(False)
    G.add_edges(G.edges()[1], G.edges()[0])
    return G


def find_connected_nodes(G):
    nodes = G.out_degrees().nonzero().squeeze(-1)
    return nodes


class DeepwalkDataset:
    def __init__(self, args):
        """ This class has the following functions:
        1. Transform the txt network file into DGL graph;
        2. Generate random walk sequences for the trainer;
        3. Provide the negative table if the user hopes to sample negative
        nodes according to nodes' degrees;

        Parameter
        ---------
        net_file str : path of the txt network file
        walk_length int : number of nodes in a sequence
        window_size int : context window size
        num_walks int : number of walks for each node
        batch_size int : number of node sequences in each batch
        negative int : negative samples for each positve node pair
        fast_neg bool : whether do negative sampling inside a batch
        """
        self.walk_length = args.walk_length
        self.window_size = args.window_size
        self.num_walks = args.num_walks
        self.batch_size = args.batch_size
        self.negative = args.negative
        self.num_procs = len(args.gpus)
        self.fast_neg = args.fast_neg

        if args.load_type == "ogbl":
            assert len(args.gpus) == 1, "ogb.linkproppred is not compatible with multi-gpu training (CUDA error)."
            from load_dataset import load_from_ogbl_with_name
            self.G = load_from_ogbl_with_name(args.ogbl_name)
            self.G = make_undirected(self.G)
        elif args.load_type == "tugraph":
            self.net, self.node2id, self.id2node, self.sm = ReadTugrpahNet(args)
            print("after load from tugraph")
            self.save_mapping(args.map_file)
            self.G = net2graph(self.sm)
        elif args.load_type == "txt":
            self.net, self.node2id, self.id2node, self.sm = ReadTxtNet(args.net_file)
            self.save_mapping(args.map_file)
            self.G = net2graph(self.sm)
        else:
            print("ERROR: unsupported load type!")
            exit(0)

        self.num_nodes = self.G.number_of_nodes()

        # random walk seeds
        start = time.time()
        self.valid_seeds = find_connected_nodes(self.G)
        if len(self.valid_seeds) != self.num_nodes:
            print("WARNING: The node ids are not serial. Some nodes are invalid.")

        seeds = torch.cat([torch.LongTensor(self.valid_seeds)] * self.num_walks)
        self.seeds = torch.split(shuffle_walks(seeds),
                                 int(np.ceil(len(self.valid_seeds) * self.num_walks / self.num_procs)),
                                 0)
        end = time.time()
        t = end - start
        print("%d seeds in %.2fs" % (len(seeds), t))

        # negative table for true negative sampling
        if not args.fast_neg:
            node_degree = self.G.out_degrees(self.valid_seeds).numpy()
            node_degree = np.power(node_degree, 0.75)
            node_degree /= np.sum(node_degree)
            node_degree = np.array(node_degree * 1e8, dtype=np.int)
            self.neg_table = []

            for idx, node in enumerate(self.valid_seeds):
                self.neg_table += [node] * node_degree[idx]
            self.neg_table_size = len(self.neg_table)
            self.neg_table = np.array(self.neg_table, dtype=np.long)
            del node_degree

    def create_sampler(self, i):
        """ create random walk sampler """
        return DeepwalkSampler(self.G, self.seeds[i], self.walk_length)

    def save_mapping(self, map_file):
        """ save the mapping dict that maps node IDs to embedding indices """
        with open(map_file, "wb") as f:
            pickle.dump(self.node2id, f)


class DeepwalkSampler(object):
    def __init__(self, G, seeds, walk_length):
        """ random walk sampler

        Parameter
        ---------
        G dgl.Graph : the input graph
        seeds torch.LongTensor : starting nodes
        walk_length int : walk length
        """
        self.G = G
        self.seeds = seeds
        self.walk_length = walk_length

    def sample(self, seeds):
        walks = dgl.sampling.random_walk(self.G, seeds, length=self.walk_length - 1)[0]
        return walks


def init_emb2pos_index(walk_length, window_size, batch_size):
    ''' select embedding of positive nodes from a batch of node embeddings

    Return
    ------
    index_emb_posu torch.LongTensor : the indices of u_embeddings
    index_emb_posv torch.LongTensor : the indices of v_embeddings

    Usage
    -----
    # emb_u.shape: [batch_size * walk_length, dim]
    batch_emb2posu = torch.index_select(emb_u, 0, index_emb_posu)
    '''
    idx_list_u = []
    idx_list_v = []
    for b in range(batch_size):
        for i in range(walk_length):
            for j in range(i - window_size, i):
                if j >= 0:
                    idx_list_u.append(j + b * walk_length)
                    idx_list_v.append(i + b * walk_length)
            for j in range(i + 1, i + 1 + window_size):
                if j < walk_length:
                    idx_list_u.append(j + b * walk_length)
                    idx_list_v.append(i + b * walk_length)

    # [num_pos * batch_size]
    index_emb_posu = torch.LongTensor(idx_list_u)
    index_emb_posv = torch.LongTensor(idx_list_v)

    return index_emb_posu, index_emb_posv


def init_emb2neg_index(walk_length, window_size, negative, batch_size):
    '''select embedding of negative nodes from a batch of node embeddings
    for fast negative sampling

    Return
    ------
    index_emb_negu torch.LongTensor : the indices of u_embeddings
    index_emb_negv torch.LongTensor : the indices of v_embeddings

    Usage
    -----
    # emb_u.shape: [batch_size * walk_length, dim]
    batch_emb2negu = torch.index_select(emb_u, 0, index_emb_negu)
    '''
    idx_list_u = []
    for b in range(batch_size):
        for i in range(walk_length):
            for j in range(i - window_size, i):
                if j >= 0:
                    idx_list_u += [i + b * walk_length] * negative
            for j in range(i + 1, i + 1 + window_size):
                if j < walk_length:
                    idx_list_u += [i + b * walk_length] * negative

    idx_list_v = list(range(batch_size * walk_length)) \
                 * negative * window_size * 2
    random.shuffle(idx_list_v)
    idx_list_v = idx_list_v[:len(idx_list_u)]

    # [bs * walk_length * negative]
    index_emb_negu = torch.LongTensor(idx_list_u)
    index_emb_negv = torch.LongTensor(idx_list_v)

    return index_emb_negu, index_emb_negv


def init_weight(walk_length, window_size, batch_size):
    ''' init context weight '''
    weight = []
    for b in range(batch_size):
        for i in range(walk_length):
            for j in range(i - window_size, i):
                if j >= 0:
                    weight.append(1. - float(i - j - 1) / float(window_size))
            for j in range(i + 1, i + 1 + window_size):
                if j < walk_length:
                    weight.append(1. - float(j - i - 1) / float(window_size))

    # [num_pos * batch_size]
    return torch.Tensor(weight).unsqueeze(1)


def init_empty_grad(emb_dimension, walk_length, batch_size):
    """ initialize gradient matrix """
    grad_u = torch.zeros((batch_size * walk_length, emb_dimension))
    grad_v = torch.zeros((batch_size * walk_length, emb_dimension))

    return grad_u, grad_v


def adam(grad, state_sum, nodes, lr, device, only_gpu):
    """ calculate gradients according to adam """
    grad_sum = (grad * grad).mean(1)
    if not only_gpu:
        grad_sum = grad_sum.cpu()
    state_sum.index_add_(0, nodes, grad_sum)  # cpu
    std = state_sum[nodes].to(device)  # gpu
    std_values = std.sqrt_().add_(1e-10).unsqueeze(1)
    grad = (lr * grad / std_values)  # gpu

    return grad


def async_update(num_threads, model, queue):
    """ asynchronous embedding update """
    torch.set_num_threads(num_threads)
    while True:
        (grad_u, grad_v, grad_v_neg, nodes, neg_nodes) = queue.get()
        if grad_u is None:
            return
        with torch.no_grad():
            model.u_embeddings.weight.data.index_add_(0, nodes.view(-1), grad_u)
            model.v_embeddings.weight.data.index_add_(0, nodes.view(-1), grad_v)
            if neg_nodes is not None:
                model.v_embeddings.weight.data.index_add_(0, neg_nodes.view(-1), grad_v_neg)


class SkipGramModel(nn.Module):
    """ Negative sampling based skip-gram """

    def __init__(self,
                 emb_size,
                 emb_dimension,
                 walk_length,
                 window_size,
                 batch_size,
                 only_cpu,
                 only_gpu,
                 mix,
                 neg_weight,
                 negative,
                 lr,
                 lap_norm,
                 fast_neg,
                 record_loss,
                 norm,
                 use_context_weight,
                 async_update,
                 num_threads,
                 ):
        """ initialize embedding on CPU

        Paremeters
        ----------
        emb_size int : number of nodes
        emb_dimension int : embedding dimension
        walk_length int : number of nodes in a sequence
        window_size int : context window size
        batch_size int : number of node sequences in each batch
        only_cpu bool : training with CPU
        only_gpu bool : training with GPU
        mix bool : mixed training with CPU and GPU
        negative int : negative samples for each positve node pair
        neg_weight float : negative weight
        lr float : initial learning rate
        lap_norm float : weight of laplacian normalization
        fast_neg bool : do negative sampling inside a batch
        record_loss bool : print the loss during training
        norm bool : do normalizatin on the embedding after training
        use_context_weight : give different weights to the nodes in a context window
        async_update : asynchronous training
        """
        super(SkipGramModel, self).__init__()
        self.emb_size = emb_size
        self.emb_dimension = emb_dimension
        self.walk_length = walk_length
        self.window_size = window_size
        self.batch_size = batch_size
        self.only_cpu = only_cpu
        self.only_gpu = only_gpu
        self.mixed_train = mix
        self.neg_weight = neg_weight
        self.negative = negative
        self.lr = lr
        self.lap_norm = lap_norm
        self.fast_neg = fast_neg
        self.record_loss = record_loss
        self.norm = norm
        self.use_context_weight = use_context_weight
        self.async_update = async_update
        self.num_threads = num_threads

        # initialize the device as cpu
        self.device = torch.device("cpu")

        # content embedding
        self.u_embeddings = nn.Embedding(
            self.emb_size, self.emb_dimension, sparse=True)
        # context embedding
        self.v_embeddings = nn.Embedding(
            self.emb_size, self.emb_dimension, sparse=True)
        # initialze embedding
        initrange = 1.0 / self.emb_dimension
        init.uniform_(self.u_embeddings.weight.data, -initrange, initrange)
        init.constant_(self.v_embeddings.weight.data, 0)

        # lookup_table is used for fast sigmoid computing
        self.lookup_table = torch.sigmoid(torch.arange(-6.01, 6.01, 0.01))
        self.lookup_table[0] = 0.
        self.lookup_table[-1] = 1.
        if self.record_loss:
            self.logsigmoid_table = torch.log(torch.sigmoid(torch.arange(-6.01, 6.01, 0.01)))
            self.loss = []

        # indexes to select positive/negative node pairs from batch_walks
        self.index_emb_posu, self.index_emb_posv = init_emb2pos_index(
            self.walk_length,
            self.window_size,
            self.batch_size)
        self.index_emb_negu, self.index_emb_negv = init_emb2neg_index(
            self.walk_length,
            self.window_size,
            self.negative,
            self.batch_size)

        if self.use_context_weight:
            self.context_weight = init_weight(
                self.walk_length,
                self.window_size,
                self.batch_size)

        # adam
        self.state_sum_u = torch.zeros(self.emb_size)
        self.state_sum_v = torch.zeros(self.emb_size)

        # gradients of nodes in batch_walks
        self.grad_u, self.grad_v = init_empty_grad(
            self.emb_dimension,
            self.walk_length,
            self.batch_size)

    def create_async_update(self):
        """ Set up the async update subprocess.
        """
        self.async_q = Queue(1)
        self.async_p = mp.Process(target=async_update, args=(self.num_threads, self, self.async_q))
        self.async_p.start()

    def finish_async_update(self):
        """ Notify the async update subprocess to quit.
        """
        self.async_q.put((None, None, None, None, None))
        self.async_p.join()

    def share_memory(self):
        """ share the parameters across subprocesses """
        self.u_embeddings.weight.share_memory_()
        self.v_embeddings.weight.share_memory_()
        self.state_sum_u.share_memory_()
        self.state_sum_v.share_memory_()

    def set_device(self, gpu_id):
        """ set gpu device """
        self.device = torch.device("cuda:%d" % gpu_id)
        print("The device is", self.device)
        self.lookup_table = self.lookup_table.to(self.device)
        if self.record_loss:
            self.logsigmoid_table = self.logsigmoid_table.to(self.device)
        self.index_emb_posu = self.index_emb_posu.to(self.device)
        self.index_emb_posv = self.index_emb_posv.to(self.device)
        self.index_emb_negu = self.index_emb_negu.to(self.device)
        self.index_emb_negv = self.index_emb_negv.to(self.device)
        self.grad_u = self.grad_u.to(self.device)
        self.grad_v = self.grad_v.to(self.device)
        if self.use_context_weight:
            self.context_weight = self.context_weight.to(self.device)

    def all_to_device(self, gpu_id):
        """ move all of the parameters to a single GPU """
        self.device = torch.device("cuda:%d" % gpu_id)
        self.set_device(gpu_id)
        self.u_embeddings = self.u_embeddings.cuda(gpu_id)
        self.v_embeddings = self.v_embeddings.cuda(gpu_id)
        self.state_sum_u = self.state_sum_u.to(self.device)
        self.state_sum_v = self.state_sum_v.to(self.device)

    def fast_sigmoid(self, score):
        """ do fast sigmoid by looking up in a pre-defined table """
        idx = torch.floor((score + 6.01) / 0.01).long()
        return self.lookup_table[idx]

    def fast_logsigmoid(self, score):
        """ do fast logsigmoid by looking up in a pre-defined table """
        idx = torch.floor((score + 6.01) / 0.01).long()
        return self.logsigmoid_table[idx]

    def fast_learn(self, batch_walks, neg_nodes=None):
        """ Learn a batch of random walks in a fast way. It has the following features:
            1. It calculating the gradients directly without the forward operation.
            2. It does sigmoid by a looking up table.

        Specifically, for each positive/negative node pair (i,j), the updating procedure is as following:
            score = self.fast_sigmoid(u_embedding[i].dot(v_embedding[j]))
            # label = 1 for positive samples; label = 0 for negative samples.
            u_embedding[i] += (label - score) * v_embedding[j]
            v_embedding[i] += (label - score) * u_embedding[j]

        Parameters
        ----------
        batch_walks list : a list of node sequnces
        lr float : current learning rate
        neg_nodes torch.LongTensor : a long tensor of sampled true negative nodes. If neg_nodes is None,
            then do negative sampling randomly from the nodes in batch_walks as an alternative.

        Usage example
        -------------
        batch_walks = [torch.LongTensor([1,2,3,4]),
                       torch.LongTensor([2,3,4,2])])
        lr = 0.01
        neg_nodes = None
        """
        lr = self.lr

        # [batch_size, walk_length]
        if isinstance(batch_walks, list):
            nodes = torch.stack(batch_walks)
        elif isinstance(batch_walks, torch.LongTensor):
            nodes = batch_walks
        if self.only_gpu:
            nodes = nodes.to(self.device)
            if neg_nodes is not None:
                neg_nodes = neg_nodes.to(self.device)
        emb_u = self.u_embeddings(nodes).view(-1, self.emb_dimension).to(self.device)
        emb_v = self.v_embeddings(nodes).view(-1, self.emb_dimension).to(self.device)

        ## Postive
        bs = len(batch_walks)
        if bs < self.batch_size:
            index_emb_posu, index_emb_posv = init_emb2pos_index(
                self.walk_length,
                self.window_size,
                bs)
            index_emb_posu = index_emb_posu.to(self.device)
            index_emb_posv = index_emb_posv.to(self.device)
        else:
            index_emb_posu = self.index_emb_posu
            index_emb_posv = self.index_emb_posv

        # num_pos: the number of positive node pairs generated by a single walk sequence
        # [batch_size * num_pos, dim]
        emb_pos_u = torch.index_select(emb_u, 0, index_emb_posu)
        emb_pos_v = torch.index_select(emb_v, 0, index_emb_posv)

        pos_score = torch.sum(torch.mul(emb_pos_u, emb_pos_v), dim=1)
        pos_score = torch.clamp(pos_score, max=6, min=-6)
        # [batch_size * num_pos, 1]
        score = (1 - self.fast_sigmoid(pos_score)).unsqueeze(1)
        if self.record_loss:
            self.loss.append(torch.mean(self.fast_logsigmoid(pos_score)).item())

        # [batch_size * num_pos, dim]
        if self.lap_norm > 0:
            grad_u_pos = score * emb_pos_v + self.lap_norm * (emb_pos_v - emb_pos_u)
            grad_v_pos = score * emb_pos_u + self.lap_norm * (emb_pos_u - emb_pos_v)
        else:
            grad_u_pos = score * emb_pos_v
            grad_v_pos = score * emb_pos_u

        if self.use_context_weight:
            if bs < self.batch_size:
                context_weight = init_weight(
                    self.walk_length,
                    self.window_size,
                    bs).to(self.device)
            else:
                context_weight = self.context_weight
            grad_u_pos *= context_weight
            grad_v_pos *= context_weight

        # [batch_size * walk_length, dim]
        if bs < self.batch_size:
            grad_u, grad_v = init_empty_grad(
                self.emb_dimension,
                self.walk_length,
                bs)
            grad_u = grad_u.to(self.device)
            grad_v = grad_v.to(self.device)
        else:
            self.grad_u = self.grad_u.to(self.device)
            self.grad_u.zero_()
            self.grad_v = self.grad_v.to(self.device)
            self.grad_v.zero_()
            grad_u = self.grad_u
            grad_v = self.grad_v
        grad_u.index_add_(0, index_emb_posu, grad_u_pos)
        grad_v.index_add_(0, index_emb_posv, grad_v_pos)

        ## Negative
        if bs < self.batch_size:
            index_emb_negu, index_emb_negv = init_emb2neg_index(
                self.walk_length, self.window_size, self.negative, bs)
            index_emb_negu = index_emb_negu.to(self.device)
            index_emb_negv = index_emb_negv.to(self.device)
        else:
            index_emb_negu = self.index_emb_negu
            index_emb_negv = self.index_emb_negv
        emb_neg_u = torch.index_select(emb_u, 0, index_emb_negu)

        if neg_nodes is None:
            emb_neg_v = torch.index_select(emb_v, 0, index_emb_negv)
        else:
            emb_neg_v = self.v_embeddings.weight[neg_nodes].to(self.device)

        # [batch_size * walk_length * negative, dim]
        neg_score = torch.sum(torch.mul(emb_neg_u, emb_neg_v), dim=1)
        neg_score = torch.clamp(neg_score, max=6, min=-6)
        # [batch_size * walk_length * negative, 1]
        score = - self.fast_sigmoid(neg_score).unsqueeze(1)
        if self.record_loss:
            self.loss.append(self.negative * self.neg_weight * torch.mean(self.fast_logsigmoid(-neg_score)).item())

        grad_u_neg = self.neg_weight * score * emb_neg_v
        grad_v_neg = self.neg_weight * score * emb_neg_u

        grad_u.index_add_(0, index_emb_negu, grad_u_neg)
        if neg_nodes is None:
            grad_v.index_add_(0, index_emb_negv, grad_v_neg)

        ## Update
        nodes = nodes.view(-1)

        # use adam optimizer
        grad_u = adam(grad_u, self.state_sum_u, nodes, lr, self.device, self.only_gpu)
        grad_v = adam(grad_v, self.state_sum_v, nodes, lr, self.device, self.only_gpu)
        if neg_nodes is not None:
            grad_v_neg = adam(grad_v_neg, self.state_sum_v, neg_nodes, lr, self.device, self.only_gpu)

        if self.mixed_train:
            grad_u = grad_u.cpu()
            grad_v = grad_v.cpu()
            if neg_nodes is not None:
                grad_v_neg = grad_v_neg.cpu()
            else:
                grad_v_neg = None

            if self.async_update:
                grad_u.share_memory_()
                grad_v.share_memory_()
                nodes.share_memory_()
                if neg_nodes is not None:
                    neg_nodes.share_memory_()
                    grad_v_neg.share_memory_()
                self.async_q.put((grad_u, grad_v, grad_v_neg, nodes, neg_nodes))

        if not self.async_update:
            self.u_embeddings.weight.data.index_add_(0, nodes.view(-1), grad_u)
            self.v_embeddings.weight.data.index_add_(0, nodes.view(-1), grad_v)
            if neg_nodes is not None:
                self.v_embeddings.weight.data.index_add_(0, neg_nodes.view(-1), grad_v_neg)
        return

    def forward(self, pos_u, pos_v, neg_v):
        ''' Do forward and backward. It is designed for future use. '''
        emb_u = self.u_embeddings(pos_u)
        emb_v = self.v_embeddings(pos_v)
        emb_neg_v = self.v_embeddings(neg_v)

        score = torch.sum(torch.mul(emb_u, emb_v), dim=1)
        score = torch.clamp(score, max=6, min=-6)
        score = -F.logsigmoid(score)

        neg_score = torch.bmm(emb_neg_v, emb_u.unsqueeze(2)).squeeze()
        neg_score = torch.clamp(neg_score, max=6, min=-6)
        neg_score = -torch.sum(F.logsigmoid(-neg_score), dim=1)

        # return torch.mean(score + neg_score)
        return torch.sum(score), torch.sum(neg_score)

    def save_embedding(self, dataset, file_name):
        """ Write embedding to local file. Only used when node ids are numbers.

        Parameter
        ---------
        dataset DeepwalkDataset : the dataset
        file_name str : the file name
        """
        embedding = self.u_embeddings.weight.cpu().data.numpy()
        if self.norm:
            embedding /= np.sqrt(np.sum(embedding * embedding, 1)).reshape(-1, 1)
        np.save(file_name, embedding)

    def save_embedding_pt(self, dataset, file_name):
        """ For ogb leaderboard.
        """
        try:
            max_node_id = max(dataset.node2id.keys())
            if max_node_id + 1 != self.emb_size:
                print("WARNING: The node ids are not serial.")

            embedding = torch.zeros(max_node_id + 1, self.emb_dimension)
            index = torch.LongTensor(list(map(lambda id: dataset.id2node[id], list(range(self.emb_size)))))
            embedding.index_add_(0, index, self.u_embeddings.weight.cpu().data)

            if self.norm:
                embedding /= torch.sqrt(torch.sum(embedding.mul(embedding), 1) + 1e-6).unsqueeze(1)
            torch.save(embedding, file_name)
        except:
            self.save_embedding_pt_dgl_graph(dataset, file_name)

    def save_embedding_pt_dgl_graph(self, dataset, file_name):
        """ For ogb leaderboard """
        embedding = torch.zeros_like(self.u_embeddings.weight.cpu().data)
        valid_seeds = torch.LongTensor(dataset.valid_seeds)
        valid_embedding = self.u_embeddings.weight.cpu().data.index_select(0,
                                                                           valid_seeds)
        embedding.index_add_(0, valid_seeds, valid_embedding)

        if self.norm:
            embedding /= torch.sqrt(torch.sum(embedding.mul(embedding), 1) + 1e-6).unsqueeze(1)

        torch.save(embedding, file_name)

    def save_embedding_txt(self, dataset, file_name):
        """ Write embedding to local file. For future use.

        Parameter
        ---------
        dataset DeepwalkDataset : the dataset
        file_name str : the file name
        """
        embedding = self.u_embeddings.weight.cpu().data.numpy()
        if self.norm:
            embedding /= np.sqrt(np.sum(embedding * embedding, 1)).reshape(-1, 1)
        with open(file_name, 'w') as f:
            f.write('%d %d\n' % (self.emb_size, self.emb_dimension))
            for wid in range(self.emb_size):
                e = ' '.join(map(lambda x: str(x), embedding[wid]))
                f.write('%s %s\n' % (str(dataset.id2node[wid]), e))


class DeepwalkTrainer:
    def __init__(self, args):
        """ Initializing the trainer with the input arguments """
        self.args = args
        self.dataset = DeepwalkDataset(args)
        self.emb_size = self.dataset.G.number_of_nodes()
        self.emb_model = None

    def init_device_emb(self):
        """ set the device before training
        will be called once in fast_train_mp / fast_train
        """
        choices = sum([self.args.only_gpu, self.args.only_cpu, self.args.mix])
        assert choices == 1, "Must choose only *one* training mode in [only_cpu, only_gpu, mix]"

        # initializing embedding on CPU
        self.emb_model = SkipGramModel(
            emb_size=self.emb_size,
            emb_dimension=self.args.dim,
            walk_length=self.args.walk_length,
            window_size=self.args.window_size,
            batch_size=self.args.batch_size,
            only_cpu=self.args.only_cpu,
            only_gpu=self.args.only_gpu,
            mix=self.args.mix,
            neg_weight=self.args.neg_weight,
            negative=self.args.negative,
            lr=self.args.lr,
            lap_norm=self.args.lap_norm,
            fast_neg=self.args.fast_neg,
            record_loss=self.args.print_loss,
            norm=self.args.norm,
            use_context_weight=self.args.use_context_weight,
            async_update=self.args.async_update,
            num_threads=self.args.num_threads,
        )

        torch.set_num_threads(self.args.num_threads)
        if self.args.only_gpu:
            print("Run in 1 GPU")
            assert self.args.gpus[0] >= 0
            self.emb_model.all_to_device(self.args.gpus[0])
        elif self.args.mix:
            print("Mix CPU with %d GPU" % len(self.args.gpus))
            if len(self.args.gpus) == 1:
                assert self.args.gpus[0] >= 0, 'mix CPU with GPU should have available GPU'
                self.emb_model.set_device(self.args.gpus[0])
        else:
            print("Run in CPU process")
            self.args.gpus = [torch.device('cpu')]

    def train(self):
        """ train the embedding """
        if len(self.args.gpus) > 1:
            self.fast_train_mp()
        else:
            self.fast_train()

    def fast_train_mp(self):
        """ multi-cpu-core or mix cpu & multi-gpu """
        self.init_device_emb()
        self.emb_model.share_memory()

        if self.args.count_params:
            sum_up_params(self.emb_model)

        start_all = time.time()
        ps = []

        for i in range(len(self.args.gpus)):
            p = mp.Process(target=self.fast_train_sp, args=(i, self.args.gpus[i]))
            ps.append(p)
            p.start()

        for p in ps:
            p.join()

        print("Used time: %.2fs" % (time.time() - start_all))
        if self.args.save_in_txt:
            self.emb_model.save_embedding_txt(self.dataset, self.args.output_emb_file)
        elif self.args.save_in_pt:
            self.emb_model.save_embedding_pt(self.dataset, self.args.output_emb_file)
        else:
            self.emb_model.save_embedding(self.dataset, self.args.output_emb_file)

    def fast_train_sp(self, rank, gpu_id):
        """ a subprocess for fast_train_mp """
        if self.args.mix:
            self.emb_model.set_device(gpu_id)

        torch.set_num_threads(self.args.num_threads)
        if self.args.async_update:
            self.emb_model.create_async_update()

        sampler = self.dataset.create_sampler(rank)

        dataloader = DataLoader(
            dataset=sampler.seeds,
            batch_size=self.args.batch_size,
            collate_fn=sampler.sample,
            shuffle=False,
            drop_last=False,
            num_workers=self.args.num_sampler_threads,
        )
        num_batches = len(dataloader)
        print("num batchs: %d in process [%d] GPU [%d]" % (num_batches, rank, gpu_id))
        # number of positive node pairs in a sequence
        num_pos = int(2 * self.args.walk_length * self.args.window_size \
                      - self.args.window_size * (self.args.window_size + 1))

        start = time.time()
        with torch.no_grad():
            for i, walks in enumerate(dataloader):
                if self.args.fast_neg:
                    self.emb_model.fast_learn(walks)
                else:
                    # do negative sampling
                    bs = len(walks)
                    neg_nodes = torch.LongTensor(
                        np.random.choice(self.dataset.neg_table,
                                         bs * num_pos * self.args.negative,
                                         replace=True))
                    self.emb_model.fast_learn(walks, neg_nodes=neg_nodes)

                if i > 0 and i % self.args.print_interval == 0:
                    if self.args.print_loss:
                        print("GPU-[%d] batch %d time: %.2fs loss: %.4f" \
                              % (gpu_id, i, time.time() - start, -sum(self.emb_model.loss) / self.args.print_interval))
                        self.emb_model.loss = []
                    else:
                        print("GPU-[%d] batch %d time: %.2fs" % (gpu_id, i, time.time() - start))
                    start = time.time()

            if self.args.async_update:
                self.emb_model.finish_async_update()

    def fast_train(self):
        """ fast train with dataloader with only gpu / only cpu"""
        # the number of postive node pairs of a node sequence
        num_pos = 2 * self.args.walk_length * self.args.window_size \
                  - self.args.window_size * (self.args.window_size + 1)
        num_pos = int(num_pos)

        self.init_device_emb()

        if self.args.async_update:
            self.emb_model.share_memory()
            self.emb_model.create_async_update()

        if self.args.count_params:
            sum_up_params(self.emb_model)

        sampler = self.dataset.create_sampler(0)

        dataloader = DataLoader(
            dataset=sampler.seeds,
            batch_size=self.args.batch_size,
            collate_fn=sampler.sample,
            shuffle=False,
            drop_last=False,
            num_workers=self.args.num_sampler_threads,
        )

        num_batches = len(dataloader)
        print("num batchs: %d\n" % num_batches)

        start_all = time.time()
        start = time.time()
        with torch.no_grad():
            max_i = num_batches
            for i, walks in enumerate(dataloader):
                if self.args.fast_neg:
                    self.emb_model.fast_learn(walks)
                else:
                    # do negative sampling
                    bs = len(walks)
                    neg_nodes = torch.LongTensor(
                        np.random.choice(self.dataset.neg_table,
                                         bs * num_pos * self.args.negative,
                                         replace=True))
                    self.emb_model.fast_learn(walks, neg_nodes=neg_nodes)

                if i > 0 and i % self.args.print_interval == 0:
                    if self.args.print_loss:
                        print("Batch %d training time: %.2fs loss: %.4f" \
                              % (i, time.time() - start, -sum(self.emb_model.loss) / self.args.print_interval))
                        self.emb_model.loss = []
                    else:
                        print("Batch %d, training time: %.2fs" % (i, time.time() - start))
                    start = time.time()

            if self.args.async_update:
                self.emb_model.finish_async_update()

        print("Training used time: %.2fs" % (time.time() - start_all))
        if self.args.save_in_txt:
            self.emb_model.save_embedding_txt(self.dataset, self.args.output_emb_file)
        elif self.args.save_in_pt:
            self.emb_model.save_embedding_pt(self.dataset, self.args.output_emb_file)
        else:
            self.emb_model.save_embedding(self.dataset, self.args.output_emb_file)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="DeepWalk")
    # input files
    ## personal datasets
    parser.add_argument('--load_type', type=str, default="tugraph",
                        help="load type")
    parser.add_argument('--data_file', type=str, default="./db_cora",
                        help="path of the txt network file or tugraph database directory")
    parser.add_argument("--username", type=str, default="admin",
                        help="database username")
    parser.add_argument("--password", type=str,
                        help="database password")
    parser.add_argument("--graph_name", type=str, default="default",
                        help="import graph name")
    parser.add_argument("--make_undirected", default=True, action="store_true",
                        help="whether to make graph undirected")

    ## ogbl datasets
    parser.add_argument('--ogbl_name', type=str,
                        help="name of ogbl dataset, e.g. ogbl-ddi")
    parser.add_argument('--load_from_ogbl', default=False, action="store_true",
                        help="whether load dataset from ogbl")

    # output files
    parser.add_argument('--save_in_txt', default=False, action="store_true",
                        help='Whether save dat in txt format or npy')
    parser.add_argument('--save_in_pt', default=False, action="store_true",
                        help='Whether save dat in pt format or npy')
    parser.add_argument('--output_emb_file', type=str, default="emb.npy",
                        help='path of the output npy embedding file')
    parser.add_argument('--map_file', type=str, default="nodeid_to_index.pickle",
                        help='path of the mapping dict that maps node ids to embedding index')
    parser.add_argument('--norm', default=False, action="store_true",
                        help="whether to do normalization over node embedding after training")

    # model parameters
    parser.add_argument('--dim', default=128, type=int,
                        help="embedding dimensions")
    parser.add_argument('--window_size', default=5, type=int,
                        help="context window size")
    parser.add_argument('--use_context_weight', default=False, action="store_true",
                        help="whether to add weights over nodes in the context window")
    parser.add_argument('--num_walks', default=10, type=int,
                        help="number of walks for each node")
    parser.add_argument('--negative', default=1, type=int,
                        help="negative samples for each positve node pair")
    parser.add_argument('--batch_size', default=128, type=int,
                        help="number of node sequences in each batch")
    parser.add_argument('--walk_length', default=80, type=int,
                        help="number of nodes in a sequence")
    parser.add_argument('--neg_weight', default=1., type=float,
                        help="negative weight")
    parser.add_argument('--lap_norm', default=0.01, type=float,
                        help="weight of laplacian normalization, recommend to set as 0.1 / windoe_size")

    # training parameters
    parser.add_argument('--print_interval', default=100, type=int,
                        help="number of batches between printing")
    parser.add_argument('--print_loss', default=False, action="store_true",
                        help="whether print loss during training")
    parser.add_argument('--lr', default=0.2, type=float,
                        help="learning rate")

    # optimization settings
    parser.add_argument('--mix', default=False, action="store_true",
                        help="mixed training with CPU and GPU")
    parser.add_argument('--gpus', type=int, default=[-1], nargs='+',
                        help='a list of active gpu ids, e.g. 0, used with --mix')
    parser.add_argument('--only_cpu', default=False, action="store_true",
                        help="training with CPU")
    parser.add_argument('--only_gpu', default=False, action="store_true",
                        help="training with GPU")
    parser.add_argument('--async_update', default=False, action="store_true",
                        help="mixed training asynchronously, not recommended")

    parser.add_argument('--true_neg', default=False, action="store_true",
                        help="If not specified, this program will use "
                             "a faster negative sampling method, "
                             "but the samples might be false negative "
                             "with a small probability. If specified, "
                             "this program will generate a true negative sample table,"
                             "and select from it when doing negative samling")
    parser.add_argument('--num_threads', default=8, type=int,
                        help="number of threads used for each CPU-core/GPU")
    parser.add_argument('--num_sampler_threads', default=2, type=int,
                        help="number of threads used for sampling")

    parser.add_argument('--count_params', default=False, action="store_true",
                        help="count the params, exit once counting over")

    args = parser.parse_args()
    args.fast_neg = not args.true_neg
    if args.async_update:
        assert args.mix, "--async_update only with --mix"

    start_time = time.time()
    trainer = DeepwalkTrainer(args)
    trainer.train()
    print("Total used time: %.2f" % (time.time() - start_time))
