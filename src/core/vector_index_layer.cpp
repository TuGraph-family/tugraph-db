#include "core/vector_index_layer.h"

namespace lgraph {
VectorIndex::VectorIndex(const std::string& label, const std::string& name, const std::string& distance_type, const std::string& index_type, int vec_dimension, std::vector<int> index_spec, std::shared_ptr<KvTable> table)
    : label_(label), name_(name), distance_type_(distance_type), index_type_(index_type), 
      vec_dimension_(vec_dimension), index_spec_(index_spec),
      query_spec_(10), quantizer_(nullptr), index_(nullptr),
      vector_index_manager_(size_t(0), label, name), table_(std::move(table)), rebuild_(false) {}

VectorIndex::VectorIndex(const VectorIndex& rhs)
    : label_(rhs.label_), 
      name_(rhs.name_), 
      distance_type_(rhs.distance_type_),
      index_type_(rhs.index_type_),
      vec_dimension_(rhs.vec_dimension_), 
      index_spec_(rhs.index_spec_),
      query_spec_{rhs.query_spec_}, 
      quantizer_(rhs.quantizer_),
      index_(rhs.index_),
      vector_index_manager_(rhs.vector_index_manager_),
      table_(rhs.table_),
      rebuild_(rhs.rebuild_) {}

bool VectorIndex::SetSearchSpec(int query_spec) {
    query_spec_ = query_spec;
    return true;
}

// add vector to index
bool VectorIndex::Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors) {
    //reduce dimension
    std::vector<float> index_vectors;
    index_vectors.reserve(num_vectors * vec_dimension_);
    for (const auto& vec : vectors) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    // TODO
    if (index_type_ == "IVF_FLAT") {
        // train after build quantizer
        assert(!index_->is_trained);
        index_->train(num_vectors, index_vectors.data());
        assert(index_->is_trained);
        index_->add(num_vectors, index_vectors.data());
    } else {
        return false;
    }
    return true;
}

// build index
bool VectorIndex::Build() {
    // TODO
    if (index_type_ == "IVF_FLAT") {
        quantizer_ = new faiss::IndexFlatL2(vec_dimension_);
        index_ = new faiss::IndexIVFFlat(quantizer_, vec_dimension_, index_spec_[0]);
    } else {
        return false;
    }
    return true;
}

// serialize index
std::vector<uint8_t> VectorIndex::Save() {
    faiss::VectorIOWriter writer;
    faiss::write_index(index_, &writer, 0);
    return writer.data;
}

// load index form serialization
void VectorIndex::Load(std::vector<uint8_t>& idx_bytes) {
    faiss::VectorIOReader reader;
    reader.data = idx_bytes;
    index_ = dynamic_cast<faiss::IndexIVFFlat*>(faiss::read_index(&reader));
}

// search vector in index
bool VectorIndex::Search(const std::vector<float> query, size_t num_results, std::vector<float>& distances, std::vector<int64_t>& indices) {
    if (query.empty() || num_results == 0) {
        return false;
    }
    distances.resize(num_results * 1);
    indices.resize(num_results * 1);
    index_->nprobe = static_cast<size_t>(query_spec_);
    index_->search(1, query.data(), num_results, distances.data(), indices.data());
    return !indices.empty();
}

std::unique_ptr<KvTable> VectorIndex::OpenTable(KvTransaction& txn, KvStore& store,
                                          const std::string& name, FieldType dt, IndexType type) {
    return store.OpenTable(txn, name, true, ComparatorDesc::DefaultComparator());
}

void VectorIndex::AddVectorInTable(KvTransaction& txn, const Value& k, VertexId vid) {
    //Value key = CutKeyIfLong(k);
    table_->AppendKv(txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), k);
}

void VectorIndex::CleanVectorFromTable(KvTransaction& txn) {
    table_->Drop(txn);
    rebuild_ = true;
}

bool VectorIndex::GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query, size_t num_results, std::vector<float>& distances, std::vector<int64_t>& indices) {
    std::vector<std::vector<float>> floatvector;
    size_t count = 0;
    auto kv_iter = table_->GetIterator(txn);
    for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
        auto prop = kv_iter->GetValue();
        auto vector = prop.AsType<std::vector<float>>();
        floatvector.emplace_back(vector);
        count++;
    }
    std::vector<float> index_vectors;
    index_vectors.reserve(count * vec_dimension_);
    for (auto vec : floatvector) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    std::unique_ptr<faiss::IndexFlat> index;
    if (distance_type_ ==  "IP") {
        index = std::make_unique<faiss::IndexFlatIP>(vec_dimension_);
    } else if (distance_type_ == "L2") {
        index = std::make_unique<faiss::IndexFlatL2>(vec_dimension_);
    } else {
        return false;
    }
    index->add(count, index_vectors.data());
    distances.resize(num_results * 1);
    indices.resize(num_results * 1);
    index->search(1, query.data(), num_results, distances.data(), indices.data());
    return true;
}
}