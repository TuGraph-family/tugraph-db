# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libcpp.string import string
from cython.cimports.libcpp.vector import vector

@cython.cclass
class PyOlapOnDB:
    olapondb: cython.p_void
    edgedata: str
    vertex_label_list: list
    edge_label_list: list
    meta_label_list: list

    def __init__(self, edgedata: str, db: PyGraphDB, txn: PyTxn, schema: list = []):
        self.edgedata = edgedata
        self.vertex_label_list = []
        self.edge_label_list = []
        self.meta_label_list = []
        label_key: string
        if edgedata == "Empty":
            label_list = vector[vector[string]]()
            if len(schema) != 0:
                for i in schema:
                    labels = vector[string]()
                    for j in range(3):
                        label_key = i[j].encode("utf-8")
                        if j == 1 and i[j] not in self.edge_label_list:
                            for e_label in txn.txn.ListEdgeLabels():
                                if label_key == e_label:
                                    self.edge_label_list.append(i[j])
                                    break
                            else:
                                print("%s label not found" % i[j])
                        elif i[j] not in self.vertex_label_list:
                            for v_label in txn.txn.ListVertexLabels():
                                if label_key == v_label:
                                    self.vertex_label_list.append(i[j])
                                    break
                            else:
                                print("%s label not found" % i[j])
                        labels.push_back(label_key)
                    self.meta_label_list.append(i)
                    label_list.push_back(labels)
                self.olapondb = new OlapOnDB[Empty](db.db, txn.txn, label_list, SNAPSHOT_PARALLEL)
            else:
                self.olapondb = new OlapOnDB[Empty](db.db, txn.txn, SNAPSHOT_PARALLEL)

    def ntypes(self):
        return self.vertex_label_list
    
    def etypes(self):
        return self.edge_label_list

    def metagraph(self):
        return self.meta_label_list

    def get_pointer(self) -> cython.Py_ssize_t:
        return cython.cast(cython.Py_ssize_t, self.olapondb)

    def __del__(self):
        if self.edgedata == "Empty":
            _olapondb = cython.cast(cython.pointer(OlapOnDB[Empty]), self.olapondb)
            del _olapondb

@cython.cclass
class PyTxn:
    txn: Transaction
    def __init__(self, db: PyGraphDB, if_read_only: bool):
        if if_read_only:
            self.txn = db.db.CreateReadTxn()
        else:
            self.txn = db.db.CreateWriteTxn()
    def GetNumVertices(self):
        return self.txn.GetNumVertices()
    def Commit(self):
        self.txn.Commit()
    def ListVertexLabels(self):
        self.txn.ListVertexLabels()
    def ListEdgeLabels(self):
        self.txn.ListEdgeLabels()

@cython.cclass
class PyGraphDB:
    db: GraphDB

    def __init__(self, galaxy: PyGalaxy, graph: str, read_only: bool):
        self.db = galaxy.galaxy.OpenGraph(graph.encode('utf-8'), read_only)

    def get_pointer(self) -> cython.Py_ssize_t:
        return cython.cast(cython.Py_ssize_t, cython.address(self.db))

    def CreateReadTxn(self):
        return PyTxn(self, True)
    def CreateWriteTxn(self):
        return PyTxn(self, False)


@cython.cclass
class PyGalaxy:
    galaxy: Galaxy

    def __init__(self, path: str):
        self.galaxy = Galaxy(path.encode('utf-8'))

    def SetCurrentUser(self, user: str, password: str):
        self.galaxy.SetCurrentUser(user.encode('utf-8'), password.encode('utf-8'))

    def SetUser(self, user: str):
        self.galaxy.SetUser(user.encode('utf-8'))

    def OpenGraph(self, graph: str, read_only: bool):
        return PyGraphDB(self, graph, read_only)