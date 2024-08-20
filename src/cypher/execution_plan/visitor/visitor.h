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

#pragma once

namespace cypher {
class OpBase;
class Aggregate;
class AllNodeScan;
class AllNodeScanDynamic;
class Apply;
class Argument;
class CartesianProduct;
class OpCreate;
class OpGqlCreate;
class OpDelete;
class OpGqlDelete;
class Distinct;
class ExpandAll;
class OpFilter;
class InQueryCall;
class GqlInQueryCall;
class Limit;
class OpMerge;
class OpGqlMerge;
class NodeByIdSeek;
class NodeByLabelScan;
class NodeByLabelScanDynamic;
class NodeIndexSeek;
class NodeIndexSeekDynamic;
class Optional;
class ProduceResults;
class Project;
class RelationshipCount;
class OpRemove;
class OpGqlRemove;
class OpSet;
class OpGqlSet;
class Skip;
class Sort;
class StandaloneCall;
class OpGqlStandaloneCall;
class TopN;
class Union;
class Unwind;
class VarLenExpand;
class VarLenExpandInto;
class Traversal;
class OpGqlTraversal;
// nested op
class ImmediateArgument;
}  // namespace cypher

namespace cypher {

// cyclic visitor
class Visitor {
 public:
    virtual ~Visitor() = default;
    virtual void Visit(const OpBase &op) = 0;
    virtual void Visit(const Aggregate &op) = 0;
    virtual void Visit(const AllNodeScan &op) = 0;
    virtual void Visit(const AllNodeScanDynamic &op) = 0;
    virtual void Visit(const Apply &op) = 0;
    virtual void Visit(const Argument &op) = 0;
    virtual void Visit(const CartesianProduct &op) = 0;
    virtual void Visit(const OpCreate &op) = 0;
    virtual void Visit(const OpDelete &op) = 0;
    virtual void Visit(const Distinct &op) = 0;
    virtual void Visit(const ExpandAll &op) = 0;
    virtual void Visit(const OpFilter &op) = 0;
    virtual void Visit(const InQueryCall &op) = 0;
    virtual void Visit(const Limit &op) = 0;
    virtual void Visit(const OpMerge &op) = 0;
    virtual void Visit(const NodeByIdSeek &op) = 0;
    virtual void Visit(const NodeByLabelScan &op) = 0;
    virtual void Visit(const NodeByLabelScanDynamic &op) = 0;
    virtual void Visit(const NodeIndexSeek &op) = 0;
    virtual void Visit(const NodeIndexSeekDynamic &op) = 0;
    virtual void Visit(const Optional &op) = 0;
    virtual void Visit(const ProduceResults &op) = 0;
    virtual void Visit(const Project &op) = 0;
    virtual void Visit(const RelationshipCount &op) = 0;
    virtual void Visit(const OpRemove &op) = 0;
    virtual void Visit(const OpSet &op) = 0;
    virtual void Visit(const Skip &op) = 0;
    virtual void Visit(const Sort &op) = 0;
    virtual void Visit(const StandaloneCall &op) = 0;
    virtual void Visit(const TopN &op) = 0;
    virtual void Visit(const Union &op) = 0;
    virtual void Visit(const Unwind &op) = 0;
    virtual void Visit(const VarLenExpand &op) = 0;
    virtual void Visit(const VarLenExpandInto &op) = 0;
    virtual void Visit(const Traversal &op) = 0;
    // nested op
    virtual void Visit(const ImmediateArgument &op) = 0;
    // gql op
    virtual void Visit(const OpGqlCreate &op) = 0;
    virtual void Visit(const OpGqlDelete &op) = 0;
    virtual void Visit(const GqlInQueryCall &op) = 0;
    virtual void Visit(const OpGqlMerge &op) = 0;
    virtual void Visit(const OpGqlRemove &op) = 0;
    virtual void Visit(const OpGqlSet &op) = 0;
    virtual void Visit(const OpGqlStandaloneCall &op) = 0;
    virtual void Visit(const OpGqlTraversal &op) = 0;
};

#define CYPHER_DEFINE_VISITABLE() \
    void Accept(Visitor *visitor) override { return visitor->Visit(*this); }

#define CYPHER_DEFINE_CONST_VISITABLE() \
    void Accept(Visitor *visitor) const override { return visitor->Visit(*this); }

}  // namespace cypher
