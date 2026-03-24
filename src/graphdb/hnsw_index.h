#pragma once

#include <faiss/Index.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "proto/meta.pb.h"

namespace faiss {
struct IndexHNSW;
struct IndexFlatCodes;
struct Index;
}  // namespace faiss

namespace graphdb {

class FaissHnswIndex {
 public:
  using Filter = std::function<bool(int64_t)>;

  struct SearchResult {
    std::vector<int64_t> ids;
    std::vector<float> distances;
  };

  FaissHnswIndex(int64_t dim, meta::VectorDistanceType distance_type,
                 int hnsw_m, int ef_construction);

  static std::unique_ptr<FaissHnswIndex> Load(
      const std::string& path, int64_t dim,
      meta::VectorDistanceType distance_type, int hnsw_m, int ef_construction);

  void Add(const float* vectors, int64_t num_elements,
           const int64_t* ids = nullptr);

  SearchResult KnnSearch(const float* query, int top_k, int ef_search,
                         const Filter& filter = Filter()) const;

  void WriteToFile(const std::string& path) const;

  int64_t GetNumElements() const;
  int64_t GetMemoryUsage() const;

 private:
  explicit FaissHnswIndex(std::unique_ptr<faiss::Index> index,
                          meta::VectorDistanceType distance_type, int hnsw_m,
                          int ef_construction);

  static int DistanceTypeToFaissMetricType(
      meta::VectorDistanceType distance_type);

  faiss::Index* BaseIndex();
  const faiss::Index* BaseIndex() const;
  faiss::IndexHNSW* HnswIndex();
  const faiss::IndexHNSW* HnswIndex() const;

  faiss::IndexFlatCodes* FlatStorage();
  const faiss::IndexFlatCodes* FlatStorage() const;

  void ValidateIndex(int64_t dim) const;

  std::unique_ptr<faiss::Index> index_;
  meta::VectorDistanceType distance_type_;
  int hnsw_m_;
  int ef_construction_;
};

}  // namespace graphdb
