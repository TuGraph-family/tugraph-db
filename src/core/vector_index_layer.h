#pragma once

#include <vector>
#include <cstdint>
#include "core/vector_index_manager.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexIVFFlat.h"
#include "core/kv_store.h"
#include "core/graph_data_pack.h"
#include "faiss/index_io.h"
#include "faiss/impl/io.h"

namespace lgraph {

class VectorIndex {
   friend class Schema;
   friend class LightningGraph;
   friend class Transaction;
   friend class IndexManager;

   std::string label_;
   std::string name_;
   std::string distance_type_;
   std::string index_type_;
   int vec_dimension_;
   std::vector<int> index_spec_;
   int query_spec_;
   faiss::IndexFlatL2* quantizer_;
   faiss::IndexIVFFlat* index_;
   VectorIndexManager vector_index_manager_;
   std::shared_ptr<KvTable> table_;
   bool rebuild_;

 public:
    VectorIndex(const std::string& label, const std::string& name, const std::string& distance_type,
                const std::string& index_type, int vec_dimension, std::vector<int> index_spec, std::shared_ptr<KvTable> table);

    VectorIndex(const VectorIndex& rhs);

    VectorIndex(VectorIndex&& rhs) = delete;

    VectorIndex& operator=(const VectorIndex& rhs) = delete;

    VectorIndex& operator=(VectorIndex&& rhs) = delete;

    // get label
    std::string GetLabel() { return label_; }

    // get name
    std::string GetName() { return name_; }

    //get distance type
    std::string GetDistanceType() { return distance_type_; }

    //get index type
    std::string GetIndexType() { return index_type_; }

    //get vector dimension
    int GetVecDimension() { return vec_dimension_; }

    //get the vector table
    std::shared_ptr<KvTable> GetTable() { return table_; }

    // get vector_index_manager
    VectorIndexManager* GetVectorIndexManager() { return &vector_index_manager_; }

    // get whether rebuild
    bool GetWhetherRebuild() { return rebuild_; }

    //set search specification
    bool SetSearchSpec(int query_spec);

    // add vector to index and build index
    bool Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors);

    // build index
    bool Build();

    // serialize index
    std::vector<uint8_t> Save();

    // load index form serialization
    void Load(std::vector<uint8_t>& idx_bytes);

    // search vector in index
    bool Search(const std::vector<float> query, size_t num_results, std::vector<float>& distances, std::vector<int64_t>& indices);

    static std::unique_ptr<KvTable> OpenTable(KvTransaction& txn, KvStore& store,
                                   const std::string& name, FieldType dt, IndexType type);

    void AddVectorInTable(KvTransaction& txn, const Value& k, VertexId vid);

    void CleanVectorFromTable(KvTransaction& txn);

    bool GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query, size_t num_results, std::vector<float>& distances, std::vector<int64_t>& indices);

};
} // namespace lgraph