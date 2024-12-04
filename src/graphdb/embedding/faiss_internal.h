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
 *
 *  Author:
 *      Junwang Zhao <zhaojunwang.zjw@antgroup.com>
 */

#pragma once

#include <faiss/Index.h>
#include <faiss/MetricType.h>
#include <faiss/impl/IDSelector.h>

#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

namespace {

faiss::MetricType DistanceTypeToFaissMetricType(
    meta::VectorDistanceType distance_type) {
    switch (distance_type) {
        case meta::VectorDistanceType::L2:
            return faiss::MetricType::METRIC_L2;
        // cosine can be converted to IP, so faiss do not have COSINE metric
        case meta::VectorDistanceType::IP:
        case meta::VectorDistanceType::COSINE:
            return faiss::MetricType::METRIC_INNER_PRODUCT;
    }
}

}  // namespace

class IdSelector;

struct FaissIDSelector : public faiss::IDSelector {
   public:
    FaissIDSelector(const IdSelector& sel) : sel_(sel) {}
    virtual ~FaissIDSelector() = default;

    bool is_member(int64_t id) const override;

   private:
    const IdSelector& sel_;
};

}  // namespace embedding

}  // namespace graphdb
