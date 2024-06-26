#pragma once

#include <vector>
#include <cstdint>
#include "core/vector_index_manager.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexIVFFlat.h"
#include "faiss/index_io.h"
#include "faiss/impl/io.h"

namespace lgraph {

class VectorIndex {
   friend class Schema;
   friend class LightningGraph;
   friend class Transaction;
   friend class IndexManager;

 private:
    std::string label_;
    std::string name_;
    std::string distance_type_;
    std::string index_type_;
    int vec_dimension_;
    std::vector<int> index_spec_;
    int query_spec_;
    size_t num_of_return_;
    faiss::IndexFlatL2* quantizer_;
    faiss::Index* index_;
    VectorIndexManager vector_index_manager_; 

 public:
    VectorIndex(const std::string& label, const std::string& name, const std::string& distance_type, const std::string& index_type, int vec_dimension, std::vector<int> index_spec);

    VectorIndex(const VectorIndex& rhs);

    VectorIndex(VectorIndex&& rhs) = delete;

    VectorIndex& operator=(const VectorIndex& rhs) = delete;

    VectorIndex& operator=(VectorIndex&& rhs) = delete;
    
    ~VectorIndex();

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

    // get vector_index_manager
    VectorIndexManager GetVectorIndexManager() { return vector_index_manager_; }

    //set search specification
    bool SetSearchSpec(int query_spec);

    //set return number of vector
    bool SetReturnNum(size_t return_num);

    // add vector to index and build index
    bool Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors);

    // build index
    bool Build();

    // serialize index
    std::vector<uint8_t> Save();

    // load index form serialization
    void Load(std::vector<uint8_t>& idx_bytes);

    // search vector in index
    bool Search(const std::vector<float> query, size_t num_results, size_t near_results, std::vector<std::vector<float>>& vector_results);

    // get IndexIVFFlat
    faiss::IndexIVFFlat* GetIndexIVFFlat() { return dynamic_cast<faiss::IndexIVFFlat*>(index_); }
};
}  // namespace lgraph