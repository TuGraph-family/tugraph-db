# distutils: language = c++

from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.pair cimport pair

ctypedef size_t size_t
ctypedef unsigned long uint64_t
ctypedef unsigned int uint32_t
ctypedef signed long int64_t
ctypedef signed int int32_t
ctypedef short int int16_t
ctypedef unsigned short int	uint16_t

cdef extern from "../../include/lgraph/lgraph_types.h" namespace "lgraph_api":
    cdef struct EdgeUid:
        int64_t src
        int64_t dst
        uint16_t lid
        int64_t tid
        int64_t eid
        # EdgeUid()
        EdgeUid(int64_t s, int64_t d, uint16_t l, int64_t t, int64_t e)
        void Reverse()
        # bint operator==(const EdgeUid& rhs)
        string ToString()


cdef extern from "../../include/lgraph/lgraph_vertex_index_iterator.h" namespace "lgraph_api":
    cdef cppclass VertexIndexIterator nogil:
        int64_t GetVid() const

cdef extern from "../../include/lgraph/lgraph_vertex_iterator.h" namespace "lgraph_api":
    cdef cppclass VertexIterator nogil:
        pass

cdef extern from "../../include/lgraph/lgraph_edge_iterator.h" namespace "lgraph_api":
    cdef cppclass OutEdgeIterator nogil:
        pass

    cdef cppclass InEdgeIterator nogil:
            pass

cdef extern from "../../include/lgraph/lgraph_galaxy.h" namespace "lgraph_api":
    cdef cppclass Galaxy nogil:
        Galaxy(const string & dir_path, bint durable, bint create_if_not_exist)
        Galaxy(const string & dir_path, bint durable)
        Galaxy(const string & dir_path)
        Galaxy(const string& dir, const string& user, const string& password,
                bint durable, bint create_if_not_exist)
        void SetCurrentUser(const string & user, const string& password)
        void SetUser(const string &user)
        bint CreateGraph(const string& graph_name, const string& description,
                             size_t max_size)
        bint CreateGraph(const string& graph_name, const string& description)
        bint CreateGraph(const string& graph_name)
        bint DeleteGraph(const string& graph_name)
        GraphDB OpenGraph(const string & graph, bint read_only)
        bint ModGraph(const string& graph_name, bint mod_desc, const string& desc,
                        bint mod_size, size_t new_max_size)
        map[string, pair[string, size_t]] ListGraphs()
        bint CreateUser(const string& user, const string& password,
                           const string& desc)
        bint CreateUser(const string& user, const string& password)
        bint DeleteUser(const string& user)
        bint SetPassword(const string& user, const string& old_password,
                             const string& new_password)
        bint SetUserDesc(const string& user, const string& desc)
        bint SetUserRoles(const string& user, const vector[string]& roles)
        # bint SetUserGraphAccess(const string& user, const string& graph,
        #                            const AccessLevel& access)
        bint DisableUser(const string& user)
        bint EnableUser(const string& user)
        # map[string, UserInfo] ListUsers()
        # UserInfo GetUserInfo(const string& user)
        bint CreateRole(const string& role, const string& desc)
        bint DeleteRole(const string& role)
        bint DisableRole(const string& role)
        bint EnableRole(const string& role)
        bint SetRoleDesc(const string& role, const string& desc)
        # bint SetRoleAccessRights(const string& role,
        #                            const map[string, AccessLevel]& graph_access)
        # bint SetRoleAccessRightsIncremental(const string& role,
        #                                        const map[string, AccessLevel]& graph_access)
        # RoleInfo GetRoleInfo(const string& role)
        # map[string, RoleInfo] ListRoles()
        # AccessLevel GetAccessLevel(const string& user, const string& graph)
        void Close()


cdef extern from "../../include/lgraph/lgraph_txn.h" namespace "lgraph_api":
    cdef cppclass Transaction nogil:
        VertexIndexIterator GetVertexIndexIterator(const string& label, const string& field, const string& key_start, const string& key_end)
        size_t GetNumVertices()
        void Commit()
        void Abort()
        bint IsValid()
        bint IsReadOnly()
        # const shared_ptr<lgraph::Transaction> GetTxn()
        VertexIterator GetVertexIterator()
        VertexIterator GetVertexIterator(int64_t vid)
        VertexIterator GetVertexIterator(int64_t vid, bint nearest)
        OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid)
        OutEdgeIterator GetOutEdgeIterator(const EdgeUid& euid, bint nearest)
        OutEdgeIterator GetOutEdgeIterator(const int64_t src, const int64_t dst, const int16_t lid)
        InEdgeIterator GetInEdgeIterator(const EdgeUid& euid, bint nearest)
        InEdgeIterator GetInEdgeIterator(const EdgeUid& euid)
        InEdgeIterator GetInEdgeIterator(const int64_t src, const int64_t dst, const int16_t lid)
        size_t GetNumVertexLabels()
        size_t GetNumEdgeLabels()
        vector[string] ListVertexLabels()
        vector[string] ListEdgeLabels()
        size_t GetVertexLabelId(const string& label)
        size_t GetEdgeLabelId(const string& label)
        # vector[FieldSpec] GetVertexSchema(const string& label)
        # vector[FieldSpec] GetEdgeSchema(const string& label)
        size_t GetVertexFieldId(size_t label_id, const string& field_name)
        vector[size_t] GetVertexFieldIds(size_t label_id, const vector[string]& field_names)
        size_t GetEdgeFieldId(size_t label_id, const string& field_name)
        vector[size_t] GetEdgeFieldIds(size_t label_id, const vector[string]& field_names)
        int64_t AddVertex(const string& label_name, const vector[string]& field_names,
                              const vector[string]& field_value_strings)


cdef extern from "../../include/lgraph/lgraph_db.h" namespace "lgraph_api":
    cdef cppclass GraphDB nogil:
        void Close()
        Transaction CreateReadTxn()
        Transaction CreateWriteTxn()
        Transaction CreateWriteTxn(bint optimistic)
        Transaction ForkTxn(Transaction & txn)
