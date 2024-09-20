/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include "core/vector_index.h"

namespace lgraph {
VectorIndex::VectorIndex(const std::string& label, const std::string& name,
                         const std::string& distance_type, const std::string& index_type,
                         int vec_dimension, std::vector<int> index_spec)
    : label_(label), name_(name), distance_type_(distance_type), index_type_(index_type),
      vec_dimension_(vec_dimension), index_spec_(index_spec) {}

VectorIndex::VectorIndex(const VectorIndex& rhs)
    : label_(rhs.label_),
      name_(rhs.name_),
      distance_type_(rhs.distance_type_),
      index_type_(rhs.index_type_),
      vec_dimension_(rhs.vec_dimension_),
      index_spec_(rhs.index_spec_) {}

}  // namespace lgraph
