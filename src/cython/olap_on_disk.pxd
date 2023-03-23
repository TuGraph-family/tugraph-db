# distutils: language = c++
from lgraph_db cimport *
from olap_base cimport *


cdef extern from "olap/olap_on_disk.h" namespace "lgraph_api::olap" :
    cdef cppclass OlapOnDisk[EdgeData](OlapBase[EdgeData]) nogil:
        OlapOnDisk() nogil
        void Load(ConfigBase[EdgeData] config, int edge_direction_policy) nogil

cdef extern from "olap/olap_config.h" namespace "lgraph_api::olap" :
    tuple[size_t, bint] parse_line_weighted[EdgeData](const char* p, const char* end, EdgeUnit[EdgeData]& e) nogil
    tuple[size_t, bint] parse_line_unweighted[EdgeData](const char* p, const char* end, EdgeUnit[EdgeData]& e) nogil
    cdef cppclass ConfigBase[EdgeData] nogil:
        ConfigBase() nogil
        ConfigBase(int &argc, char** &argv) nogil
        void Print() nogil
        string input_dir
        string output_dir
        tuple[size_t, bint] parse_line(const char* p, const char* end, EdgeUnit[EdgeData]& e)
