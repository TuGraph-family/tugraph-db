import torch as th
import dgl
import time
import dgl.data
from ogb.nodeproppred import DglNodePropPredDataset
# from torch_sparse import SparseTensor

binary_op_dict = {
    'add': lambda x, y: x + y,
    'sub': lambda x, y: x - y,
    'mul': lambda x, y: x * y,
    'div': lambda x, y: x / y,
    'dot': lambda x, y: (x * y).sum(-1),
    'copy_u': lambda x, y: x,
    'copy_e': lambda x, y: y
}

class th_op_time(object):
    def __enter__(self):
        if th.cuda.is_available():
            self.start_event = th.cuda.Event(enable_timing=True)
            self.end_event = th.cuda.Event(enable_timing=True)
            self.start_event.record()
        else:
            self.tic = time.time()
        return self

    def __exit__(self, type, value, traceback):
        if th.cuda.is_available():
            self.end_event.record()
            th.cuda.synchronize()  # Wait for the events to be recorded!
            self.time = self.start_event.elapsed_time(self.end_event) / 1e3
        else:
            self.time = time.time() - self.tic


def homo_to_hetero(g):
    if not isinstance(g, dgl.DGLHeteroGraph):
        return dgl.graph(g.edges())
    return g


def get_graph(dataset):
    if dataset == 'reddit':
        reddit = dgl.data.RedditDataset()
        return homo_to_hetero(reddit[0])
    elif dataset == 'arxiv':
        arxiv = DglNodePropPredDataset(name='ogbn-arxiv')
        return homo_to_hetero(arxiv[0][0])
    elif dataset == 'proteins':
        protein = DglNodePropPredDataset(name='ogbn-proteins')
        return homo_to_hetero(protein[0][0])
    else:
        raise KeyError("Unrecognized dataset name: {}".format(dataset))
