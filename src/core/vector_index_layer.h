#pragma once

#include <vector>
#include <cstdint>

namespace lgraph {

class VectorIndex {
 public:
    virtual ~VectorIndex() = default;

    // add vector to index
    virtual void Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors) = 0;

    // build index
    virtual void Build() = 0;

    // serialize index
    virtual std::vector<uint8_t> Save() = 0;

    // load index form serialization
    virtual void Load(std::vector<uint8_t>& idx_bytes) = 0;

    // search vector in index
    virtual std::vector<std::vector<float>> Search(const std::vector<float>& query, size_t num_results) = 0;
};
}  // namespace vectorindex