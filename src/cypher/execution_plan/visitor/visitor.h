/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
class OpDelete;
class Distinct;
class ExpandAll;
class OpFilter;
class InQueryCall;
class Limit;
class OpMerge;
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
class OpSet;
class Skip;
class Sort;
class StandaloneCall;
class TopN;
class Union;
class Unwind;
class VarLenExpand;
class VarLenExpandInto;
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
    // nested op
    virtual void Visit(const ImmediateArgument &op) = 0;
};

#define CYPHER_DEFINE_VISITABLE() \
    void Accept(Visitor *visitor) override { return visitor->Visit(*this); }

#define CYPHER_DEFINE_CONST_VISITABLE() \
    void Accept(Visitor *visitor) const override { return visitor->Visit(*this); }

}  // namespace cypher
