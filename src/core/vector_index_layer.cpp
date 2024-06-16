#include "core/vector_index_layer.h"

namespace lgraph {
VectorIndex::VectorIndex(const std::string& label, const std::string& name, const std::string& distance_type, const std::string& index_type, int vec_dimension, std::vector<int> index_spec)
    : label_(label), name_(name), distance_type_(distance_type), index_type_(index_type), 
      vec_dimension_(vec_dimension), index_spec_(index_spec),
      query_spec_{10}, num_of_return_(1), index_(),
      vector_index_manager_(size_t(0), label, name) {}

VectorIndex::VectorIndex(const VectorIndex& rhs)
    : label_(rhs.label_), 
      name_(rhs.name_), 
      distance_type_(rhs.distance_type_),
      index_type_(rhs.index_type_),
      vec_dimension_(rhs.vec_dimension_), 
      index_spec_(rhs.index_spec_),
      query_spec_{rhs.query_spec_}, 
      num_of_return_(rhs.num_of_return_),
      index_(rhs.index_),
      vector_index_manager_(rhs.vector_index_manager_) {}

bool VectorIndex::SetSearchSpec(int query_spec) {
    query_spec_ = query_spec;
    return true;
}

bool VectorIndex::SetReturnNum(size_t return_num) {
    num_of_return_ = return_num;
    return true;
}

// add vector to index
bool VectorIndex::Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors) {
    return true;
}

// build index
bool VectorIndex::Build() {
    return true;
}

// serialize index
std::vector<uint8_t> VectorIndex::Save() {
    std::vector<uint8_t> emptyVector;  
    return emptyVector;
}

// load index form serialization
void VectorIndex::Load(std::vector<uint8_t>& idx_bytes) {

}

// search vector in index
bool VectorIndex::Search(const std::vector<float> query, size_t num_results, std::vector<std::vector<float>>& vector_results, std::vector<float> score_results) {
    return true;
}
}