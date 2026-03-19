/**
 * Copyright 2024 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "graphdb/hnsw_index.h"

#include <faiss/IndexFlatCodes.h>
#include <faiss/IndexHNSW.h>
#include <faiss/impl/FaissException.h>
#include <faiss/impl/HNSW.h>
#include <faiss/impl/IDSelector.h>
#include <faiss/index_io.h>

#include <algorithm>
#include <filesystem>

#include "common/exceptions.h"

namespace graphdb {

namespace {

class CallbackIdSelector : public faiss::IDSelector {
 public:
  explicit CallbackIdSelector(const FaissHnswIndex::Filter& filter)
      : filter_(filter) {}

  bool is_member(faiss::idx_t id) const override { return !filter_(id); }

 private:
  const FaissHnswIndex::Filter& filter_;
};

}  // namespace

FaissHnswIndex::FaissHnswIndex(int64_t dim,
                               meta::VectorDistanceType distance_type,
                               int hnsw_m, int ef_construction)
    : FaissHnswIndex(std::unique_ptr<faiss::Index>(new faiss::IndexHNSWFlat(
                         dim, hnsw_m,
                         static_cast<faiss::MetricType>(
                             DistanceTypeToFaissMetricType(distance_type)))),
                     distance_type, hnsw_m, ef_construction) {}

FaissHnswIndex::FaissHnswIndex(std::unique_ptr<faiss::Index> index,
                               meta::VectorDistanceType distance_type,
                               int hnsw_m, int ef_construction)
    : index_(std::move(index)),
      distance_type_(distance_type),
      hnsw_m_(hnsw_m),
      ef_construction_(ef_construction) {
  auto* hnsw = HnswIndex();
  hnsw->hnsw.efConstruction = ef_construction_;
}

std::unique_ptr<FaissHnswIndex> FaissHnswIndex::Load(
    const std::string& path, int64_t dim,
    meta::VectorDistanceType distance_type, int hnsw_m, int ef_construction) {
  try {
    auto* raw_index = faiss::read_index(path.c_str());
    std::unique_ptr<FaissHnswIndex> index(
        new FaissHnswIndex(std::unique_ptr<faiss::Index>(raw_index),
                           distance_type, hnsw_m, ef_construction));
    index->ValidateIndex(dim);
    return index;
  } catch (const faiss::FaissException& e) {
    THROW_CODE(IOException, "failed to load faiss hnsw index {}: {}", path,
               e.msg);
  }
  return nullptr;
}

void FaissHnswIndex::Add(const float* vectors, int64_t num_elements) {
  if (num_elements <= 0) {
    return;
  }

  try {
    index_->add(num_elements, vectors);
  } catch (const faiss::FaissException& e) {
    THROW_CODE(VectorIndexException, "failed to add vectors: {}", e.msg);
  }
}

FaissHnswIndex::SearchResult FaissHnswIndex::KnnSearch(
    const float* query, int top_k, int ef_search, const Filter& filter) const {
  SearchResult result;
  if (top_k <= 0) {
    return result;
  }

  result.ids.assign(top_k, -1);
  result.distances.assign(top_k, 0);

  faiss::SearchParametersHNSW params;
  params.efSearch = std::max(ef_search, top_k);
  CallbackIdSelector selector(filter);
  if (filter) {
    params.sel = &selector;
  }

  try {
    index_->search(1, query, top_k, result.distances.data(), result.ids.data(),
                   &params);
  } catch (const faiss::FaissException& e) {
    THROW_CODE(VectorIndexException, "search failed with exception: {}", e.msg);
  }

  return result;
}

void FaissHnswIndex::WriteToFile(const std::string& path) const {
  std::filesystem::path target(path);
  std::filesystem::path tmp =
      target.parent_path() / (target.filename().string() + ".tmp");

  try {
    faiss::write_index(index_.get(), tmp.c_str());
    std::filesystem::rename(tmp, target);
  } catch (const faiss::FaissException& e) {
    THROW_CODE(IOException, "failed to persist faiss hnsw index {}: {}", path,
               e.msg);
  } catch (const std::filesystem::filesystem_error& e) {
    THROW_CODE(IOException, "failed to persist faiss hnsw index {}: {}", path,
               e.what());
  }
}

int64_t FaissHnswIndex::GetNumElements() const { return index_->ntotal; }

int64_t FaissHnswIndex::GetMemoryUsage() const {
  const auto* hnsw = HnswIndex();
  int64_t bytes = sizeof(*hnsw);
  bytes += hnsw->hnsw.assign_probas.capacity() * sizeof(double);
  bytes += hnsw->hnsw.cum_nneighbor_per_level.capacity() * sizeof(int);
  bytes += hnsw->hnsw.levels.capacity() * sizeof(int);
  bytes += hnsw->hnsw.offsets.capacity() * sizeof(size_t);
  bytes += hnsw->hnsw.neighbors.capacity() * sizeof(faiss::HNSW::storage_idx_t);

  const auto* storage = FlatStorage();
  bytes += storage->codes.capacity() * sizeof(uint8_t);
  return bytes;
}

int FaissHnswIndex::DistanceTypeToFaissMetricType(
    meta::VectorDistanceType distance_type) {
  switch (distance_type) {
    case meta::VectorDistanceType::L2:
      return faiss::MetricType::METRIC_L2;
    case meta::VectorDistanceType::IP:
    case meta::VectorDistanceType::COSINE:
      return faiss::MetricType::METRIC_INNER_PRODUCT;
    default:
      THROW_CODE(VectorIndexException, "invalid metric_type: {}",
                 meta::VectorDistanceType_Name(distance_type));
  }
  return faiss::MetricType::METRIC_L2;
}

faiss::IndexHNSW* FaissHnswIndex::HnswIndex() {
  auto* hnsw = dynamic_cast<faiss::IndexHNSW*>(index_.get());
  if (hnsw == nullptr) {
    THROW_CODE(VectorIndexException, "loaded index is not a faiss hnsw index");
  }
  return hnsw;
}

const faiss::IndexHNSW* FaissHnswIndex::HnswIndex() const {
  auto* hnsw = dynamic_cast<const faiss::IndexHNSW*>(index_.get());
  if (hnsw == nullptr) {
    THROW_CODE(VectorIndexException, "loaded index is not a faiss hnsw index");
  }
  return hnsw;
}

faiss::IndexFlatCodes* FaissHnswIndex::FlatStorage() {
  auto* storage = dynamic_cast<faiss::IndexFlatCodes*>(HnswIndex()->storage);
  if (storage == nullptr) {
    THROW_CODE(VectorIndexException,
               "faiss hnsw index storage is not flat code storage");
  }
  return storage;
}

const faiss::IndexFlatCodes* FaissHnswIndex::FlatStorage() const {
  auto* storage =
      dynamic_cast<const faiss::IndexFlatCodes*>(HnswIndex()->storage);
  if (storage == nullptr) {
    THROW_CODE(VectorIndexException,
               "faiss hnsw index storage is not flat code storage");
  }
  return storage;
}

void FaissHnswIndex::ValidateIndex(int64_t dim) const {
  if (index_->d != dim) {
    THROW_CODE(VectorIndexException,
               "dimension mismatch when loading faiss hnsw index, expect {}, "
               "actual {}",
               dim, index_->d);
  }
  if (index_->metric_type !=
      static_cast<faiss::MetricType>(
          DistanceTypeToFaissMetricType(distance_type_))) {
    THROW_CODE(VectorIndexException,
               "metric mismatch when loading faiss hnsw index");
  }

  const auto* hnsw = HnswIndex();
  if (hnsw->hnsw.nb_neighbors(0) != hnsw_m_ * 2 ||
      hnsw->hnsw.nb_neighbors(1) != hnsw_m_) {
    THROW_CODE(VectorIndexException,
               "hnsw m mismatch when loading faiss hnsw index");
  }
  if (hnsw->hnsw.efConstruction != ef_construction_) {
    THROW_CODE(VectorIndexException,
               "hnsw ef_construction mismatch when loading faiss hnsw index");
  }
}

}  // namespace graphdb
