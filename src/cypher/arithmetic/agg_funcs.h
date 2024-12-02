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

//
// Created by wt on 19-2-15.
//
#pragma once

#include "cypher/cypher_types.h"
#include "cypher/arithmetic/agg_ctx.h"

namespace cypher {

namespace _detail {
/* Record Entry -> double number */
static bool Entry2Double(const Entry &e, double &d) {
    if (e.type != Entry::CONSTANT) CYPHER_TODO();
    if (e.IsInteger()) {
        d = static_cast<double>(e.constant.scalar.integer());
    } else if (e.IsReal()) {
        d = e.constant.scalar.real();
    } else {
        return false;
    }
    return true;
}
}  // namespace _detail

class SumAggCtx : public AggCtx {
    size_t num = 0;
    double total = 0;

 public:
    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        double n;
        for (size_t i = 1; i < args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                err = "SUM Could not convert upstream value to double";
                return 1;
            }
            num++;
            total += n;
        }
        return 0;
    }

    int ReduceNext() override {
        // TODO(anyone) handle int64_t
        result.type = Entry::CONSTANT;
        result.constant = lgraph::FieldData(total);
        // Reset(); ?
        return 0;
    }
};

class AvgAggCtx : public AggCtx {
    size_t count = 0;
    double total = 0;

 public:
    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        double n;
        for (size_t i = 1; i < args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                err = "AVG Could not convert upstream value to double";
                return 1;
            }
            count++;
            total += n;
        }
        return 0;
    }

    int ReduceNext() override {
        if (count > 0) {
            result = Entry(cypher::FieldData(lgraph::FieldData(total / count)));
        } else {
            result = Entry(cypher::FieldData());
        }
        return 0;
    }
};

class MaxAggCtx : public AggCtx {
    size_t count = 0;
    double max = std::numeric_limits<double>::lowest();

 public:
    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        double n;
        for (size_t i = 1; i < args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                err = "MAX Could not convert upstream value to double";
                return 1;
            }
            if (n > max) max = n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        if (count > 0) {
            result = Entry(cypher::FieldData(lgraph::FieldData(max)));
        } else {
            result = Entry(cypher::FieldData());
        }
        return 0;
    }
};

class MinAggCtx : public AggCtx {
    size_t count = 0;
    double min = std::numeric_limits<double>::max();

 public:
    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        double n;
        for (size_t i = 1; i < args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                err = "MIN Could not convert upstream value to double";
                return 1;
            }
            if (n < min) min = n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        if (count > 0) {
            result = Entry(cypher::FieldData(lgraph::FieldData(min)));
        } else {
            result = Entry(cypher::FieldData());
        }
        return 0;
    }
};

class CountAggCtx : public AggCtx {
    size_t count = 0;
    int distinct = -1;
    /* count(distinct sth), used to identify unique records. */
    std::unordered_set<lgraph_api::FieldData, lgraph_api::FieldData::Hash> uset;

 public:
    int Step(const std::vector<Entry> &args) override {
        if (args.size() == 1) {
            /* count(*), note that count(distinct *) is grammatically incorrect
             * the only arg indicates whether to count in. */
            if (!args[0].EqualNull()) count++;
            return 0;
        }
        CYPHER_THROW_ASSERT(args.size() == 2 && args[0].IsBool());
        if (distinct < 0) distinct = args[0].constant.scalar.AsBool() ? 1 : 0;
        if (!args[1].EqualNull()) {
            if (distinct > 0) {
                switch (args[1].type) {
                case Entry::CONSTANT:
                    if (args[1].constant.type == cypher::FieldData::SCALAR) {
                        uset.emplace(args[1].constant.scalar);
                    } else {
                        uset.emplace(lgraph_api::FieldData(args[1].ToString()));
                    }
                    break;
                case Entry::NODE:
                    uset.emplace(lgraph_api::FieldData(args[1].node->PullVid()));
                    break;
                case Entry::RELATIONSHIP:
                    uset.emplace(lgraph_api::FieldData(args[1].ToString()));
                    break;
                default:
                    uset.emplace(lgraph_api::FieldData(args[1].ToString()));
                }
            } else {
                count++;
            }
        }
        return 0;
    }

    int ReduceNext() override {
        if (distinct > 0) {
            count = uset.size();
            uset.clear();
        }
        result.type = Entry::CONSTANT;
        result.constant = lgraph::FieldData::Int64(count);
        return 0;
    }
};

class CountStarAggCtx : public CountAggCtx {};

class CollectAggCtx : public AggCtx {
    bool distinct = false;
    std::unordered_set<std::string> uset;

 public:
    CollectAggCtx() { result = Entry(cypher::FieldData::Array(0)); }

    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() == 2 && args[0].IsBool() &&
                            args[1].type == Entry::CONSTANT);
        distinct = args[0].constant.scalar.AsBool();
        for (size_t i = 1; i < args.size(); i++) {
            if (!args[i].constant.EqualNull()) {
                if (distinct) {
                    auto ret = uset.emplace(args[i].constant.ToString());
                    if (!ret.second) continue;
                }
                if (args[i].IsArray()) {
                    result.constant.array->emplace_back(args[i].constant.ToString());
                } else {
                    result.constant.array->emplace_back(args[i].constant.scalar);
                }
            }
        }
        return 0;
    }

    int ReduceNext() override {
        uset.clear();
        return 0;
    }
};

class PercentileDiscAggCtx : public AggCtx {
    // Percentile will be updated by the first call to Step
    double percentile = -1.0;
    size_t count = 0;
    std::vector<double> values;
    size_t values_allocated;

 public:
    PercentileDiscAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        auto argc = args.size();
        CYPHER_THROW_ASSERT(argc > 2 && args[0].IsBool());
        if (percentile < 0) {
            if (!_detail::Entry2Double(args[argc - 1], percentile)) {
                err = "PERCENTILE DISC Could not convert upstream value to double";
                return 1;
            }
            if (percentile < 0 || percentile > 1) {
                err =
                    "PERCENTILE_DISC Invalid input for percentile is not a valid argument,"
                    " must be a number in the range 0.0 to 1.0";
                return 1;
            }
        }
        if (count + argc - 2 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (size_t i = 1; i < argc - 1; i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "PERCENTILE DISC Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        std::sort(values.begin(), values.begin() + count);
        CYPHER_THROW_ASSERT(count > 0);
        size_t idx = percentile > 0 ? (size_t)std::ceil(percentile * count) - 1 : 0;
        result.type = Entry::CONSTANT;
        result.constant = lgraph::FieldData(values[idx]);
        return 0;
    }
};

class PercentileContAggCtx : public AggCtx {
    // Percentile will be updated by the first call to Step
    double percentile = -1;
    size_t count = 0;
    std::vector<double> values;
    size_t values_allocated;

 public:
    PercentileContAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        auto argc = args.size();
        CYPHER_THROW_ASSERT(argc > 2 && args[0].IsBool());
        if (percentile < 0) {
            if (!_detail::Entry2Double(args[argc - 1], percentile)) {
                err = "PERCENTILE CONT Could not convert upstream value to double";
                return 1;
            }
            if (percentile < 0 || percentile > 1) {
                err =
                    "PERCENTILE CONT Invalid input for percentile is not a valid argument,"
                    " must be a number in the range 0.0 to 1.0";
                return 1;
            }
        }
        if (count + argc - 2 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (size_t i = 1; i < argc - 1; i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "PERCENTILE CONT Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        std::sort(values.begin(), values.begin() + count);
        result.type = Entry::CONSTANT;

        if (percentile == 1.0 || count == 1) {
            result.constant = lgraph::FieldData(values[count - 1]);
            return 0;
        }

        double int_val, fraction_val;
        double float_idx = percentile * (count - 1);
        // Split the temp value into its integer and fractional values
        fraction_val = std::modf(float_idx, &int_val);
        // Casting the integral part of the value to an int for convenience
        CYPHER_THROW_ASSERT(int_val >= 0);
        size_t index = static_cast<size_t>(int_val);

        if (!fraction_val) {
            // A valid index was requested, so we can directly return a value
            result.constant = lgraph::FieldData(values[index]);
            return 0;
        }

        double lhs, rhs;
        lhs = values[index] * (1 - fraction_val);
        rhs = values[index + 1] * fraction_val;
        result.constant = lgraph::FieldData(lhs + rhs);
        return 0;
    }
};

class StDevAggCtx : public AggCtx {
    size_t count = 0;
    double total = 0;
    std::vector<double> values;
    size_t values_allocated;
    int is_sampled = 1;

 public:
    StDevAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        if (count + args.size() - 1 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (int i = 1; i < (int)args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "STDEV Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            total += n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        result.type = Entry::CONSTANT;
        if (count < 2) {
            result.constant = lgraph::FieldData(0.0);
            return 0;
        }

        double mean = total / count;
        long double sum = 0;
        for (int i = 0; i < (int)count; i++) {
            sum += (values[i] - mean) * (values[i] + mean);
        }
        // is_sampled will be equal to 1 in the Stdev case and 0 in the StdevP case
        double variance = sum / (count - is_sampled);
        double stdev = std::sqrt(variance);

        result.constant = lgraph::FieldData(stdev);
        return 0;
    }
};

class StDevPAggCtx : public AggCtx {
    size_t count = 0;
    double total = 0;
    std::vector<double> values;
    size_t values_allocated;
    int is_sampled = 0;

 public:
    StDevPAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        if (count + args.size() - 1 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (int i = 1; i < (int)args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "STDEVP Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            total += n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        result.type = Entry::CONSTANT;
        if (count < 2) {
            result.constant = lgraph::FieldData(0.0);
            return 0;
        }

        double mean = total / count;
        long double sum = 0;
        for (int i = 0; i < (int)count; i++) {
            sum += (values[i] - mean) * (values[i] + mean);
        }
        // is_sampled will be equal to 1 in the Stdev case and 0 in the StdevP case
        double variance = sum / (count - is_sampled);
        double stdev = std::sqrt(variance);

        result.constant = lgraph::FieldData(stdev);
        return 0;
    }
};

// sample variance
class VarianceAggCtx : public AggCtx {
    size_t count = 0;
    double total = 0;
    std::vector<double> values;
    size_t values_allocated;
    int is_sampled = 1;

 public:
    VarianceAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        if (count + args.size() - 1 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (int i = 1; i < (int)args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "VARIANCE Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            total += n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        result.type = Entry::CONSTANT;
        if (count < 2) {
            result.constant = lgraph::FieldData(0.0);
            return 0;
        }
        double mean = total / count;
        long double sum = 0;
        for (int i = 0; i < (int)count; i++) {
            sum += (values[i] - mean) * (values[i] + mean);
        }
        // is_sampled will be equal to 1 in the Varaince case and 0 in the VarianceP case
        double variance = sum / (count - is_sampled);
        result.constant = lgraph::FieldData(variance);
        return 0;
    }
};

// population variance
class VariancePAggCtx : public AggCtx {
    size_t count = 0;
    double total = 0;
    std::vector<double> values;
    size_t values_allocated;
    int is_sampled = 0;

 public:
    VariancePAggCtx() : values_allocated(1024) { values.resize(values_allocated); }

    int Step(const std::vector<Entry> &args) override {
        CYPHER_THROW_ASSERT(args.size() > 1 && args[0].IsBool());
        if (count + args.size() - 1 > values_allocated) {
            values_allocated *= 2;
            values.resize(values_allocated);
        }
        double n;
        for (int i = 1; i < (int)args.size(); i++) {
            if (!_detail::Entry2Double(args[i], n)) {
                if (!args[i].constant.EqualNull()) {
                    // not convertible to double!
                    err = "VARIANCEP Could not convert upstream value to double";
                    return 1;
                } else {
                    return 0;
                }
            }
            values[count] = n;
            total += n;
            count++;
        }
        return 0;
    }

    int ReduceNext() override {
        result.type = Entry::CONSTANT;
        if (count < 2) {
            result.constant = lgraph::FieldData(0.0);
            return 0;
        }
        double mean = total / count;
        long double sum = 0;
        for (int i = 0; i < (int)count; i++) {
            sum += (values[i] - mean) * (values[i] + mean);
        }
        // is_sampled will be equal to 1 in the Variance case and 0 in the VarianceP case
        double variance = sum / (count - is_sampled);
        result.constant = lgraph::FieldData(variance);
        return 0;
    }
};
}  // namespace cypher
