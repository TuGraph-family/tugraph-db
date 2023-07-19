import dgl
import dgl.ops
import torch as th
import torch
from utils import th_op_time, get_graph
import argparse

from dgl.ops import gspmm

import gpc


n_cold_start = 2

def bench_spmm_helper(g, ctx, binary_op, reduce_op, n_hid):
    try:
        nfeat = th.randn(g.number_of_src_nodes(), 1, n_hid, device=ctx)
        efeat = th.randn(g.number_of_edges(), 1, 1, device=ctx) if binary_op != 'copy_lhs' else None
        accum_time = 0
        for n_times in range(10):
            with th_op_time() as timer:
                ret = dgl.ops.gspmm(g, binary_op, reduce_op, nfeat, efeat)
            if n_times >= n_cold_start:
                accum_time += timer.time
        avg_time = accum_time / (n_times - n_cold_start)
        print('hidden size: {}, avg time: {:.2}ms'.format(
            n_hid, avg_time*1000))
        return avg_time
    except Exception as e:
        print(e)
        print('hidden size: {}, OOM'.format(n_hid))

def bench_spmm(g, ctx, binary_op, reduce_op):
    print("SPMM\n----------------------------")
    with th.no_grad():
        dgls = []
        gpcs = []
        n_hids = [2, 4, 8, 16, 32, 41, 64, 128, 256]
        for n_hid in n_hids:
            dgltime = bench_spmm_helper(g, ctx, args.spmm_binary, args.spmm_reduce, n_hid)
            dgls.append(dgltime)
        for n_hid in n_hids:
            gpc.gcompile(g)
            gpctime = bench_spmm_helper(g, ctx, args.spmm_binary, args.spmm_reduce, n_hid)
            gpcs.append(gpctime)
        for i in range(len(dgls)):
            print('speedup: ', dgls[i]/gpcs[i])

def bench_spmm_grad(g, ctx, binary_op, reduce_op):
    print("SPMM\n----------------------------")
    if True:
        for n_hid in [1, 2, 4, 8, 16, 32, 64, 128]:
            try:
                nfeat = th.ones(g.number_of_src_nodes(), n_hid, device=ctx)
                efeat = th.ones(g.number_of_edges(), n_hid, device=ctx) if binary_op != 'copy_lhs' else None
                nfeat.requires_grad_()
                accum_time = 0
                for n_times in range(10):
                    with th_op_time() as timer:
                        ret = dgl.ops.gspmm(g, binary_op, reduce_op, nfeat, efeat)
                        ret.requires_grad_()
                        ret.sum().backward()
                    if n_times >= n_cold_start:
                        accum_time += timer.time
                avg_time = accum_time / (n_times - n_cold_start)
                print('hidden size: {}, avg time: {:.2}ms'.format(
                    n_hid, avg_time*1000))
            except Exception as e:
                print(e)
                print('hidden size: {}, OOM'.format(n_hid))

def bench_sddmm_helper(g, ctx, op, n_hid):
    try:
        ufeat = th.rand(g.number_of_src_nodes(), n_hid, device=ctx)
        vfeat = th.rand(g.number_of_dst_nodes(), n_hid, device=ctx)
        accum_time = 0
        for n_times in range(10):
            with th_op_time() as timer:
                dgl.ops.gsddmm(g, op, ufeat, vfeat)
            if n_times >= n_cold_start:
                accum_time += timer.time
        avg_time = accum_time / (n_times - n_cold_start)
        print('hidden size: {}, avg time: {:.2}ms'.format(
            n_hid, avg_time*1000))
        return avg_time
    except Exception as e:
        print(e)
        print('hidden size: {}, OOM'.format(n_hid))

def bench_sddmm(g, ctx, op):
    print("SDDMM\n----------------------------")
    with th.no_grad():
        dgls = []
        gpcs = []
        n_hids = [1, 2, 4, 8, 16, 32, 41, 64, 128]
        for n_hid in n_hids:
            dgltime = bench_sddmm_helper(g, ctx, op, n_hid)
            dgls.append(dgltime)
        for n_hid in n_hids:
            gpc.gcompile(g)
            gpctime = bench_sddmm_helper(g, ctx, op, n_hid)
            gpcs.append(gpctime)
        for i in range(len(dgls)):
            print('speedup: ', dgls[i]/gpcs[i])


if __name__ == '__main__':
    parser = argparse.ArgumentParser("Benchmark DGL kernels")
    # spmm ops: copy_lhs, copy_rhs, mul
    parser.add_argument('--spmm-binary', type=str, default='copy_lhs')
    parser.add_argument('--spmm-reduce', type=str, default='sum')
    # only support: 1) dot 2) other ops with len == 1.
    parser.add_argument('--sddmm-binary', type=str, default='dot')
    parser.add_argument('--gpu', '-g', type=str, default='1')
    args = parser.parse_args()
    if args.gpu == '-1':
        ctx = th.device('cpu')
    else:
        ctx = th.device(int(args.gpu))
    ctx_str = 'cpu' if args.gpu == '-1' else 'gpu'

    for dataset in ['reddit']:
        g = get_graph(dataset)
        g = g.int().to(ctx)
        print(g)
        # SPMM
        bench_spmm(g, ctx, args.spmm_binary, args.spmm_reduce)
        gpc.convert_back()
        # SDDMM
        bench_sddmm(g, ctx, args.sddmm_binary)
        del g
