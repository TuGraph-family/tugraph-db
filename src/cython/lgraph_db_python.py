# cython: language_level=3, cpp_locals=True, boundscheck=False, wraparound=False, initializedcheck=False
# distutils: language = c++

import cython
from cython.cimports.olap_base import *
from cython.cimports.lgraph_db import *
from cython.cimports.libc.stdio import printf
from cython.cimports.libcpp.string import string


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
