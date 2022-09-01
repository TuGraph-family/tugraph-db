#!/usr/bin/env python
import sys
import time
import timeit
import string
import random
from multiprocessing import Pool
from LightningGraphEmbedding import *


def random_generator(size=10, chars=string.ascii_letters + string.digits):
    return ''.join(random.choice(chars) for x in range(size))


def create_db():
    global thread_db
    thread_db = LightningGraph(db_path, True)


def read_neighbour(no):
    ret = 0
    txn = thread_db.CreateReadTxn()
    ok = True
    while True:
        try:
            vid = 0
            it = txn.GetVertexIndexIterator("person", "no", Data.genInt(no), Data.genInt(no))
            if it.IsValid():
                vid = it.GetVid()
            else:
                ok = False
            ids = []
            vit = txn.GetVertexIterator(vid)
            eit = vit.GetInEdgeIterator()
            while eit.IsValid():
                nbr = eit.GetSrc()
                ids.append(nbr)
                eit.Next()
            eit = vit.GetOutEdgeIterator()
            while eit.IsValid():
                nbr = eit.GetDst()
                ids.append(nbr)
                eit.Next()
            ret = len(ids)
            break
        except Exception, e:
            print
            e
    if not ok:
        print
        "vertex(" + no + ") not found."
    txn.Abort()
    return ret


class BenchmarkLightningGraph:
    def __clock(func):
        def clocked(self, *args):
            t0 = timeit.default_timer()
            result = func(self, *args)
            elapsed = timeit.default_timer() - t0
            name = func.__name__
            arg_str = ', '.join(repr(arg) for arg in args)
            print('%14.3f/s [%14.8fs] %s(%s)' % (self.__clock_num / elapsed, elapsed, name, arg_str))
            sys.stdout.flush()
            return result

        return clocked

    def __init__(self, db, batch, vertices=0, length=10):
        self.__db = db
        self.__batch = batch
        self.__vertices = vertices
        self.__length = length

    def __random_no(self):
        return random.randint(0, self.__vertices - 1)

    def __random_name(self):
        return random_generator(self.__length)

    def __write_vertex(self, no, name):
        txn = self.__db.CreateWriteTxn(False)
        vid = txn.AddVertex("person", ["no", "name"], [Data.genInt(no), Data.genBuf(name)])
        txn.Commit()

    def __write_edge(self, no_from, no_to):
        txn = self.__db.CreateWriteTxn(False)
        vid_from = self.__vertices
        vid_to = self.__vertices
        ok = True
        while True:
            try:
                it = txn.GetVertexIndexIterator("person", "no", Data.genInt(no_from), Data.genInt(no_from))
                if it.IsValid():
                    vid_from = it.GetVid()
                else:
                    ok = False
                it = txn.GetVertexIndexIterator("person", "no", Data.genInt(no_to), Data.genInt(no_to))
                if it.IsValid():
                    vid_to = it.GetVid()
                else:
                    ok = False
                break
            except Exception, e:
                print
                e
        if ok:
            eid = txn.AddEdge(vid_from, vid_to, "knows", [], [])
        else:
            print
            "insert of edge(" + no_from + ", " + no_to + ") faild."
        txn.Commit()

    @__clock
    def test_write_vertex(self, count):
        for i in range(count):
            no = self.__vertices
            name = self.__random_name()
            self.__vertices += 1
            self.__write_vertex(no, name)
        self.__clock_num = count

    @__clock
    def test_write_vertex_batch(self, count):
        for i in range(0, count, self.__batch):
            txn = self.__db.CreateWriteTxn(False)
            for j in range(0, self.__batch):
                no = self.__vertices
                name = self.__random_name()
                self.__vertices += 1
                vid = txn.AddVertex("person", ["no", "name"], [Data.genInt(no), Data.genBuf(name)])
            txn.Commit()
        self.__clock_num = count

    @__clock
    def test_write_edge(self, count):
        for i in range(count):
            no_from = self.__random_no()
            no_to = self.__random_no()
            self.__write_edge(no_from, no_to)
        self.__clock_num = count

    @__clock
    def test_write_edge_batch(self, count):
        for i in range(0, count, self.__batch):
            txn = self.__db.CreateWriteTxn(False)
            for j in range(0, self.__batch):
                no_from = self.__random_no()
                no_to = self.__random_no()
                vid_from = self.__vertices
                vid_to = self.__vertices
                ok = True
                while True:
                    try:
                        it = txn.GetVertexIndexIterator("person", "no", Data.genInt(no_from), Data.genInt(no_from))
                        if it.IsValid():
                            vid_from = it.GetVid()
                        else:
                            ok = False
                        it = txn.GetVertexIndexIterator("person", "no", Data.genInt(no_to), Data.genInt(no_to))
                        if it.IsValid():
                            vid_to = it.GetVid()
                        else:
                            ok = False
                        break
                    except Exception, e:
                        print
                        e
                if ok:
                    eid = txn.AddEdge(vid_from, vid_to, "knows", [], [])
                else:
                    print
                    "insert of edge(" + no_from + ", " + no_to + ") faild."
            txn.Commit()
        self.__clock_num = count

    @__clock
    def warm_up(self):
        txn = self.__db.CreateReadTxn()
        vit = txn.GetVertexIterator()
        while vit.IsValid():
            vit.Next()
        txn.Abort()
        self.__clock_num = self.__vertices

    @__clock
    def test_read_neighbour_mt(self, count):
        args = [self.__random_no() for i in range(count)]
        checksums = self.__pool.map(read_neighbour, args)
        checksum = reduce(lambda x, y: x + y, checksums)
        # print checksum
        self.__clock_num = count

    def start_mt(self, num_threads):
        self.__pool = Pool(num_threads, create_db, ())

    def stop_mt(self, num_threads):
        self.__pool.terminate()

    def num_vertices(self):
        return self.__vertices


if __name__ == "__main__":
    db_path = sys.argv[1]
    n = int(sys.argv[2])
    num_threads = int(sys.argv[3])
    batch = int(sys.argv[4])
    insertion_count = int(sys.argv[5])

    db = LightningGraph(db_path, True)

    db.AddLabel("person",
                [FieldDef("no", DataType.LBR_INT64, 0, False), FieldDef("name", DataType.LBR_STRING, 0, False)], True)
    db.AddLabel("knows", [], False)
    db.AddVertexIndex("person", "no", True)

    bm = BenchmarkLightningGraph(db, batch)

    bm.start_mt(num_threads)

    bm.test_write_vertex_batch(n)
    bm.test_write_edge_batch(n * 10)
    # bm.test_write_vertex(insertion_count)
    # bm.test_write_edge(insertion_count*10)
    # bm.warm_up()
    bm.test_read_neighbour_mt(n)
    print(bm.num_vertices())

    while True:
        bm.test_write_vertex_batch(n)
        bm.test_write_edge_batch(n * 10)
        # bm.test_write_vertex(insertion_count)
        # bm.test_write_edge(insertion_count*10)
        # bm.warm_up()
        # bm.start_mt(num_threads)
        bm.test_read_neighbour_mt(n)
        # bm.stop_mt(num_threads)
        print(bm.num_vertices())
        n *= 2
