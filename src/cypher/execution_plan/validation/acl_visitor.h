/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include "cypher/execution_plan/visitor/visitor.h"
#include "db/acl.h"

namespace cypher {
struct AccessField {
    lgraph::AclManager::LabelFieldSpec ld_spec;
    lgraph::FieldAccessLevel access_level;

    AccessField(bool is_vertex_, const std::string &label_, const std::string &field_,
                lgraph::FieldAccessLevel access_level_) {
        ld_spec.is_vertex = is_vertex_;
        ld_spec.label = label_;
        ld_spec.field = field_;
        access_level = access_level_;
    }

    bool operator==(const AccessField& other) const {
        return ld_spec.is_vertex == other.ld_spec.is_vertex
            && ld_spec.label == other.ld_spec.label
            && ld_spec.field == other.ld_spec.field
            && access_level == other.access_level;
    }
};
}  // namespace cypher

namespace std {
template <>
struct hash<cypher::AccessField> {
    size_t operator()(const cypher::AccessField &t) const noexcept {
        size_t res = 0;
        boost::hash_combine(res, t.ld_spec.is_vertex);
        boost::hash_combine(res, t.ld_spec.label);
        boost::hash_combine(res, t.ld_spec.field);
        boost::hash_combine(res, t.access_level);
        return res;
    }
};
}  // namespace std

namespace cypher {

/**
 * Demo:
 * void CollectAccessFields(const cypher::OpBase &op, ACVisitor *visitor) {
 *     op->Accept(&av);
 *     for (auto child : op.children) {
 *         CollectAccessFields(child, &av);
 *     }
 * }
 *
 * {
 *     ACVisitor av(vertex_schema, edge_schema);
 *     CollectAccessFields(op_root, av);
 * }
 * */

class ACLVisitor : public Visitor {

 public:
    explicit ACLVisitor(cypher::RTContext* ctx) : ctx_(ctx) {}
    void Traverse(const OpBase &op);
    void Visit(const OpBase &op) override;
    void Visit(const Aggregate &op) override;
    void Visit(const AllNodeScan &op) override;
    void Visit(const AllNodeScanDynamic &op) override;
    void Visit(const Apply &op) override;
    void Visit(const Argument &op) override;
    void Visit(const CartesianProduct &op) override;
    void Visit(const OpCreate &op) override;
    void Visit(const OpDelete &op) override;
    void Visit(const Distinct &op) override;
    void Visit(const ExpandAll &op) override;
    void Visit(const OpFilter &op) override;
    void Visit(const InQueryCall &op) override;
    void Visit(const Limit &op) override;
    void Visit(const OpMerge &op) override;
    void Visit(const NodeByIdSeek &op) override;
    void Visit(const NodeByLabelScan &op) override;
    void Visit(const NodeByLabelScanDynamic &op) override;
    void Visit(const NodeIndexSeek &op) override;
    void Visit(const NodeIndexSeekDynamic &op) override;
    void Visit(const Optional &op) override;
    void Visit(const ProduceResults &op) override;
    void Visit(const Project &op) override;
    void Visit(const RelationshipCount &op) override;
    void Visit(const OpRemove &op) override;
    void Visit(const OpSet &op) override;
    void Visit(const Skip &op) override;
    void Visit(const Sort &op) override;
    void Visit(const StandaloneCall &op) override;
    void Visit(const TopN &op) override;
    void Visit(const Union &op) override;
    void Visit(const Unwind &op) override;
    void Visit(const VarLenExpand &op) override;
    void Visit(const VarLenExpandInto &op) override;
    void Visit(const ImmediateArgument &op) override;

 private:
    cypher::RTContext* ctx_ = nullptr;
    std::unordered_map<std::string,
                       std::pair<SymbolNode::Type, std::string>> alias_label_;
    std::unordered_set<AccessField> visited_access_fields_;
};

}  // namespace cypher
