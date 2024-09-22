#pragma once

#include <utility>
#include "core/data_type.h"
#include "cypher/cypher_types.h"
#include "cypher/cypher_exception.h"
#include "parser/data_typedef.h"
#include "graph/node.h"
#include "graph/relationship.h"
#include "cypher/resultset/record.h"

#include "experimental/data_type/field_data.h"
using builder::dyn_var;

namespace cypher {

struct SymbolTable;
class RTContext;

namespace compilation {
struct CEntry {
    compilation::CFieldData constant_;
    cypher::Node* node_ = nullptr;
    cypher::Relationship* relationship_ = nullptr;

    enum RecordEntryType {
        UNKNOWN = 0,
        CONSTANT,
        NODE,
        RELATIONSHIP,
        VAR_LEN_RELP,
        HEADER,  // TODO(anyone) useless?
        NODE_SNAPSHOT,
        RELP_SNAPSHOT,
    } type_;

    CEntry() = default;

    explicit CEntry(const cypher::Entry& entry) {
        switch (entry.type) {
        case cypher::Entry::CONSTANT: {
            constant_ = CScalarData(entry.constant.scalar);
            type_ = CONSTANT;
            break;
        }
        case cypher::Entry::NODE: {
            node_ = entry.node;
            type_ = NODE;
            break;
        }
        case cypher::Entry::RELATIONSHIP: {
            relationship_ = entry.relationship;
            type_ = RELATIONSHIP;
            break;
        }
        default:
            CYPHER_TODO();
        }
    }

    explicit CEntry(const CFieldData &data) : constant_(data), type_(CONSTANT) {}

    explicit CEntry(CFieldData&& data) : constant_(std::move(data)), type_(CONSTANT) {}

    explicit CEntry(const CScalarData& scalar) : constant_(scalar), type_(CONSTANT) {}
};

struct CRecord { // Should be derived from cypher::Record
    std::vector<CEntry> values;
    
    CRecord() = default;

    CRecord(const cypher::Record &record) {
        for (auto& entry : record.values) {
            values.emplace_back(entry);
        }
    }
};
} // namespace compilaiton
} // namespace cypher