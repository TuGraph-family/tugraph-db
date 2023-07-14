import gpc_sparse
import dgl
import torch
import time

dgl_gspmm = dgl.backend.pytorch.sparse._gspmm
dgl_gsddmm = dgl.backend.pytorch.sparse._gsddmm


def _need_reduce_last_dim(ufeat, efeat):
    """Indicates whether to reduce the last dimension on edges
    in the backward pass of spmm,
    if so, use dot instead of mul."""
    if ufeat is None or efeat is None:
        return False
    ushp = ufeat.shape
    eshp = efeat.shape
    return ushp[1:-1] == eshp[1:-1] and eshp[-1] == 1 and ushp[-1] > 1


def _expand(x, shape):
    return x.expand(-1, *shape)


def spmm_cache_X(binary_op, reduce_op, req_grad_X, req_grad_Y):
    """Rules to identify whether to cache X in SpMM forward stage."""
    if binary_op != "copy_lhs" and req_grad_Y:
        if reduce_op == "sum":
            return True
        else:
            if binary_op == "mul":
                return True
    return False


def spmm_cache_Y(binary_op, reduce_op, req_grad_X, req_grad_Y):
    """Rules to identify whether to cache Y in SpMM forward stage."""
    if binary_op != "copy_rhs" and req_grad_X:
        if reduce_op == "sum":
            if binary_op in ["mul", "add"]:
                return True
        else:
            if binary_op == "mul":
                return True
    return False


def spmm_cache_argX(binary_op, reduce_op, req_grad_X, req_grad_Y):
    """Rules to identify whether to cache argX in SpMM forward stage."""
    if req_grad_X or req_grad_Y:
        if reduce_op in ["min", "max"]:
            return True
    return False


def spmm_cache_argY(binary_op, reduce_op, req_grad_X, req_grad_Y):
    """Rules to identify whether to cache argY in SpMM forward stage."""
    if req_grad_X or req_grad_Y:
        if reduce_op in ["min", "max"]:
            return True
    return False


class GSpMM(torch.autograd.Function):
    @staticmethod
    def forward(ctx, g, op, reduce_op, X, Y):
        # TODO save argu/e for backward
        ret = _gspmm(g, op, reduce_op, X, Y)

        ctx.backward_cache = (
            g, op, reduce_op,
            X.shape if X is not None else None,
            Y.shape if Y is not None else None,
            X.dtype if X is not None else Y.dtype,
            g.device,
            _need_reduce_last_dim(X, Y)
        )

        return ret

    @staticmethod
    def backward(ctx, dZ):
        print("Not implemented!")
        abort()


class GSDDMM(torch.autograd.Function):
    @staticmethod
    def forward(ctx, gidx, op, X, Y, lhs_target, rhs_target):
        out = _gsddmm(gidx, op, X, Y, lhs_target, rhs_target)
        return out

    @staticmethod
    def backward(ctx, dZ):
        print("Not implemented!")
        abort()


def spmm(g, binary_op, reduce_op, nfeat, efeat):
    return GSpMM.apply(g, binary_op, reduce_op, nfeat, efeat)


def sddmm(gidx, op, lhs_data, rhs_data, lhs_target='u', rhs_target='v'):
    if op == 'sub':
        op = 'add'
        rhs_data = -rhs_data
    if op == 'div':
        op = 'mul'
        rhs_data = 1. / rhs_data
    return GSDDMM.apply(gidx, op, lhs_data, rhs_data, lhs_target, rhs_target)


target_mapping = {
    'u': 0,
    'e': 1,
    'v': 2,
    'src': 0,
    'edge': 1,
    'dst': 2
}


def infer_broadcast_shape(op, shp1, shp2):
    r"""Check the shape validity, and infer the output shape given input shape and operator.
    Note the both :attr:`shp1`, :attr:`shp2` and the returned shape are feature
    shapes (i.e. we remove the first dimension, which correspond to graph statistics
    such as number of nodes, number of edges, etc.).

    We allow applying op on operands with different shapes, according to the
    broadcasting semantics of Numpy/Scipy:
    https://numpy.org/doc/stable/user/basics.broadcasting.html

    Parameters
    ----------
    op : str
        The binary op's name, could be `add`, `sub`, `mul`, `div`, `dot`, `copy_lhs`, `copy_rhs`.
    shp1 : tuple[int]
        The shape of lhs operand.
    shp2 : tuple[int]
        The shape of rhs operand.

    Returns
    -------
    tuple[int]
        shape after broadcasting
    """
    pad_shp1, pad_shp2 = shp1, shp2
    if op == "dot":
        if shp1[-1] != shp2[-1]:
            raise Error("Dot operator is only available for arrays with the "
                        "same size on last dimension, but got {} and {}."
                        .format(shp1, shp2))
    if op == "copy_lhs":
        return shp1
    if op == "copy_rhs":
        return shp2
    # operands are padded to have the same dimensionality with leading 1's.
    if len(shp1) > len(shp2):
        pad_shp2 = (1,) * (len(shp1) - len(shp2)) + shp2
    elif len(shp1) < len(shp2):
        pad_shp1 = (1,) * (len(shp2) - len(shp1)) + shp1
    for d1, d2 in zip(pad_shp1, pad_shp2):
        if d1 != d2 and d1 != 1 and d2 != 1:
            raise Error("Feature shapes {} and {} are not valid for broadcasting."
                        .format(shp1, shp2))
    rst = tuple(max(d1, d2) for d1, d2 in zip(pad_shp1, pad_shp2))
    return rst[:-1] + (1,) if op == "dot" else rst


def _gsddmm(gidx, op, lhs, rhs, lhs_target='u', rhs_target='v'):
    use_lhs = op != 'copy_rhs'
    use_rhs = op != 'copy_lhs'
    lhs_target = target_mapping[lhs_target]
    rhs_target = target_mapping[rhs_target]
    device = gidx.device
    dtype = lhs.dtype if use_lhs else rhs.dtype
    lhs_shp = lhs.shape if use_lhs else (0,)
    rhs_shp = rhs.shape if use_rhs else (0,)
    infer_shp = infer_broadcast_shape(op, lhs_shp[1:], rhs_shp[1:])
    out_shp = (gidx.number_of_edges(0), ) + infer_shp
    out = torch.empty(out_shp, dtype=dtype, device=device)

    gpc_sparse.sddmm(op,
                     list(gidx.coo_tuple),
                     lhs, rhs, out, lhs_target, rhs_target)

    return out


def _gspmm(gidx, binary_op, reduce_op, nfeat, efeat):
    csr_tuple = gidx.rcsr_tuple

    if nfeat == None:
        nfeat = torch.empty(0, 1)
    if efeat == None:
        efeat = torch.empty(0, 1)
    use_cmp = reduce_op in ['max', 'min']
    use_u = binary_op != 'copy_rhs'
    use_e = binary_op != 'copy_lhs'

    u = nfeat
    e = efeat
    ushp = u.shape if use_u else (0,)
    eshp = e.shape if use_e else (0,)
    dtype = u.dtype if use_u else e.dtype
    device = u.device if use_u else e.device
    _, dsttype = gidx.metagraph.find_edge(0)
    v_shp = (gidx.number_of_nodes(dsttype),) + infer_broadcast_shape(
        binary_op, ushp[1:], eshp[1:]
    )
    out = torch.empty(v_shp, dtype=dtype, device=device)

    arg_u, arg_e = torch.empty(0, 0, dtype=torch.int32), torch.empty(
        0, 0, dtype=torch.int32)
    if use_cmp:
        if use_u:
            arg_u = torch.empty(v_shp, dtype=torch.int32, device=device)
        if use_e:
            arg_e = torch.empty(v_shp, dtype=torch.int32, device=device)
    out_aux = [arg_u, arg_e]

    gpc_sparse.spmm(binary_op, reduce_op,
                    gidx.sorted_row,
                    list(csr_tuple),
                    nfeat, efeat, out, out_aux)

    if arg_u.shape[0] == 0:
        arg_u = None

    if arg_e.shape[0] == 0:
        arg_e = None
    return out, (arg_u, arg_e)
