from .sparse import spmm
from .sparse import sddmm
from .sparse import _gspmm
from .sparse import _gsddmm

import dgl
import dgl.ops
import sys

dgl_spmm = None
dgl_sddmm = None


dgl_reverse = None
dgl_get_item = None
dgl_get_relation_graph = None


def reverse_hook(obj, *args, **kwargs):
    # print('reverse_hook')
    def hook(obj, *args, **kwargs):
        ret = dgl_reverse(*args, **kwargs)
        setattr(ret, 'csr_tuple', obj.rcsr_tuple)
        setattr(ret, 'rcsr_tuple', obj.csr_tuple)
        setattr(ret, 'coo_tuple', obj.coo_tuple)
        setattr(ret, 'sorted_row', obj.sorted_row)
        setattr(ret, 'device', obj.device)
        return ret
    return lambda: hook(obj)

# getitem is readonly, cannot be overriden


def get_item_hook(obj, *args, **kwargs):
    def hook(obj, *args, **kwargs):
        ret = dgl_get_item(*args, **kwargs)
        setattr(ret._graph, 'csr_tuple', obj.csr_tuple)
        setattr(ret._graph, 'rcsr_tuple', obj.rcsr_tuple)
        setattr(ret._graph, 'coo_tuple', obj.coo_tuple)
        setattr(ret._graph, 'sorted_row', obj.sorted_row)
        setattr(ret._graph, 'device', obj.device)
        return ret
    return lambda: hook(obj)


def get_relation_graph_hook(obj, *args, **kwargs):
    def hook(obj, *args, **kwargs):
        gidx = dgl_get_relation_graph(*args, **kwargs)
        setattr(gidx, 'csr_tuple', obj.csr_tuple)
        setattr(gidx, 'rcsr_tuple', obj.rcsr_tuple)
        setattr(gidx, 'coo_tuple', obj.coo_tuple)
        setattr(gidx, 'sorted_row', obj.sorted_row)
        setattr(gidx, 'device', obj.device)
        setattr(gidx, 'reverse', reverse_hook(gidx))
        setattr(gidx, 'get_relation_graph', get_relation_graph_hook(gidx))
        return gidx
    return lambda x: hook(obj, x)


def gcompile(g):
    import torch
    if not torch.cuda.is_available():
        print("GPC only has speedup on CUDA gpus.")
        return
    if hasattr(g, 'compiled') == True:
        convert()
        return
    setattr(g, 'compiled', True)
    convert()
    global dgl_reverse
    global dgl_get_relation_graph
    if dgl_reverse == None:
        dgl_reverse = g._graph.reverse
    if dgl_get_relation_graph == None:
        dgl_get_relation_graph = g._graph.get_relation_graph

    import torch
    csr_tuple = g.adj_sparse('csr')
    coo_tuple = g.adj_sparse('coo')
    rcsr_tuple = g.adj_sparse('csc')
    csc_tuple = rcsr_tuple
    row_ptr = csc_tuple[0].tolist()
    rows = len(row_ptr) - 1
    row_counts = [0 for i in range(rows)]
    for i in range(rows):
        row_counts[i] = row_ptr[i+1] - row_ptr[i]
    sorted_indices = [i for i, v in sorted(
        enumerate(row_counts), key=lambda x: x[1], reverse=True)]
    sorted_row = torch.tensor(
        sorted_indices, dtype=csc_tuple[0].dtype, device=csc_tuple[0].device)

    setattr(g, 'csr_tuple', csr_tuple)
    setattr(g, 'rcsr_tuple', rcsr_tuple)
    setattr(g, 'coo_tuple', coo_tuple)
    setattr(g, 'sorted_row', sorted_row)

    setattr(g._graph, 'csr_tuple', csr_tuple)
    setattr(g._graph, 'rcsr_tuple', rcsr_tuple)
    setattr(g._graph, 'coo_tuple', coo_tuple)
    setattr(g._graph, 'sorted_row', sorted_row)
    setattr(g._graph, 'device', g.device)

    # Hook functions after set _graph.
    setattr(g._graph, 'reverse', reverse_hook(g._graph))
    setattr(g._graph, 'get_relation_graph', get_relation_graph_hook(g._graph))

    # hook g[etype], not possible because __get_item__ is read-only.
    # setattr(g, '__getitem__', get_item_hook(g._graph))


def convert_back():
    convert_back_spmm()
    convert_back_sddmm()


def convert():
    global dgl_spmm
    global dgl_sddmm
    if dgl_spmm == None:
        dgl_spmm = dgl.backend.pytorch.sparse._gspmm
    if dgl_sddmm == None:
        dgl_sddmm = dgl.backend.pytorch.sparse._gsddmm
    dgl.backend.pytorch.sparse._gspmm = _gspmm
    dgl.backend.pytorch.sparse._gsddmm = _gsddmm


def convert_back_spmm():
    dgl.backend.pytorch.sparse._gspmm = dgl_spmm


def convert_back_sddmm():
    dgl.backend.pytorch.sparse._gsddmm = dgl_sddmm


def convert3():
    pass


def _attach_zerodeg_note(docstring, reducer):
    note1 = """
    The {} function will return zero for nodes with no incoming messages.""".format(reducer)
    note2 = """
    This is implemented by replacing all {} values to zero.
    """.format("infinity" if reducer == "min" else "negative infinity")

    docstring = docstring + note1
    if reducer in ('min', 'max'):
        docstring = docstring + note2
    return docstring


def _gen_spmm_func(binary_op, reduce_op):
    name = "u_{}_e_{}".format(binary_op, reduce_op)
    docstring = """Generalized SpMM function.
    It fuses two steps into one kernel.

    1. Computes messages by {} source node and edge features.
    2. Aggregate the messages by {} as the features on destination nodes.

    Parameters
    ----------
    g : DGLHeteroGraph
        The input graph
    x : tensor
        The source node features.
    y : tensor
        The edge features.

    Returns
    -------
    tensor
        The result tensor.

    Notes
    -----
    This function supports autograd (computing input gradients given the output gradient). If the
    feature shape of two input operands do not match, we first broadcasts the features to a unified
    shape (note that the memory usage will not increase accordingly) and then performs the operation.

    Broadcasting follows NumPy semantics. Please see
    https://docs.scipy.org/doc/numpy/user/basics.broadcasting.html
    for more details about the NumPy broadcasting semantics.
    """.format(binary_op, reduce_op)
    docstring = _attach_zerodeg_note(docstring, reduce_op)

    def func(g, x, y):
        return spmm(g, binary_op, reduce_op, x, y)
    func.__name__ = name
    func.__doc__ = docstring
    return func


def _gen_copy_reduce_func(binary_op, reduce_op):

    name = "{}_{}".format(binary_op, reduce_op)
    binary_str = {
        "copy_u": "It copies node feature to edge as the message.",
        'copy_e': "It regards edge feature as message."
    }
    x_str = {
        "copy_u": "source node",
        "copy_e": "edge"
    }

    def docstring(binary_op): return _attach_zerodeg_note("""Generalized SpMM function. {}
    Then aggregates the message by {} on destination nodes.

    Parameters
    ----------
    g : DGLHeteroGraph
        The input graph
    x : tensor
        The {} features.

    Returns
    -------
    tensor
        The result tensor.

    Notes
    -----
    This function supports autograd (computing input gradients given the output gradient).
    """.format(
        binary_str[binary_op],
        reduce_op,
        x_str[binary_op]), reduce_op)

    def func(g, x):
        if binary_op == 'copy_u':
            return spmm(g, 'copy_lhs', reduce_op, x, None)
        else:
            return spmm(g, 'copy_rhs', reduce_op, None, x)

    func.__name__ = name
    func.__doc__ = docstring(binary_op)
    return func


def convert2():
    for binary_op in ["add", "sub", "mul", "div", "copy_u", "copy_e"]:
        for reduce_op in ["sum", "max", "min", "mean"]:
            if binary_op.startswith("copy"):
                func = _gen_copy_reduce_func(binary_op, reduce_op)
            else:
                func = _gen_spmm_func(binary_op, reduce_op)
            setattr(dgl.ops, func.__name__, func)
