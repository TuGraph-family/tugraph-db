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

#include "cypher/parser/expression.h"
#include "filter/filter.h"

namespace parser {
void Expression::Label2Filter() {
    CYPHER_THROW_ASSERT(type == LABEL);
    auto& l = Labels();
    std::shared_ptr<lgraph::Filter> f = std::make_shared<lgraph::LabelFilter>(
        l.first.String(), std::set<std::string>{l.second.begin(), l.second.end()});
    data = f;
    type = FILTER;
}

std::shared_ptr<Expression::EXPR_TYPE_FILTER> Expression::Filter() const {
    CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_FILTER>));
    return boost::any_cast<std::shared_ptr<EXPR_TYPE_FILTER>>(data);
}

bool Expression::operator==(const Expression& rhs) const {
    if (rhs.type != type) return false;
    switch (type) {
    case INT:
        return rhs.Int() == Int();
    case DOUBLE:
        return rhs.Double() == Double();
    case BOOL:
        return rhs.Bool() == Bool();
    case STRING:
    case VARIABLE:
    case PARAMETER:
        return rhs.String() == String();
    case LABEL:
    case PROPERTY:
        return rhs.Property().first == Property().first &&
               rhs.Property().second == Property().second;
    case MAP:
    case FUNCTION:
    case LIST:
    case FILTER:
    default:
        return false;
    }
}

bool Expression::ContainAlias(const std::unordered_set<std::string>& alias) const {
    switch (type) {
    case INT:
    case DOUBLE:
    case BOOL:
    case STRING:
        return false;
    case MAP:
        {
            for (const auto& n : Map()) {
                if (n.second.ContainAlias(alias)) {
                    return true;
                }
            }
            return false;
        }
    case VARIABLE:
        return alias.find(String()) != alias.end();
    case PARAMETER:
        return false;
    case FUNCTION:
    case MATH:
    case LIST:
        {
            for (const auto& n : List()) {
                if (n.ContainAlias(alias)) {
                    return true;
                }
            }
            return false;
        }
    case CASE:
        {
            for (const auto& n : Case().first) {
                if (n.ContainAlias(alias)) {
                    return true;
                }
            }
            return false;
        }
    case LABEL:
    case PROPERTY:
        {
            return Property().first.ContainAlias(alias);
        }
    case FILTER:
        CYPHER_TODO();
    case LIST_COMPREHENSION:
        {
            for (const auto& n : ListComprehension()) {
                if (n.ContainAlias(alias)) {
                    return true;
                }
            }
            return false;
        }
    case RELATIONSHIPS_PATTERN:
        CYPHER_TODO();
    case NULL_:
        return false;
    default:
        CYPHER_TODO();
    }
}

std::string Expression::ToString(bool pretty) const {
    std::string str;
    switch (type) {
    case INT:
        return std::to_string(Int());
    case DOUBLE:
        return std::to_string(Double());
    case BOOL:
        return Bool() ? "true" : "false";
    case STRING:
        return std::string("\"").append(String()).append("\"");
    case MAP:
        {
            auto& map = Map();
            auto size = map.size();
            int i = 0;
            if (pretty) {
                str.append("expr::map = {");
            }
            for (auto& m : map) {
                str.append("\"").append(m.first).append("\":").append(m.second.ToString());
                if (++i < (int)size) str.append(",");
            }
            if (pretty) str.append("}");
            return str;
        }
    case VARIABLE:
    case PARAMETER:
        return String();
    case FUNCTION:
        {
            /* function_name(DISTINCT_OR_NOT args) */
            auto& list = List();
            if (list.empty()) return str;
            str.append(list[0].String());
            if ((list[0].String() != "count(*)")) {
                str.append("(");
                for (int i = 1; i < (int)list.size(); i++) {
                    if (i == 1) {
                        CYPHER_THROW_ASSERT(list[1].type == BOOL);
                        if (list[1].Bool()) str.append("DISTINCT ");
                        continue;
                    }
                    if (i > 2) str.append(",");
                    str.append(list[i].ToString(pretty));
                }
                str.append(")");
            }
            return str;
        }
    case MATH:
        if (pretty) str = "expr::math = {";
    case LIST:
        {
            auto& list = List();
            if (pretty && str.empty()) str = "expr::list = {";
            for (int i = 0; i < (int)list.size(); i++) {
                if (i > 0) str.append(",");
                str.append(list[i].ToString());
            }
            if (pretty) str.append("}");
            return str;
        }
    case CASE:
        {
            auto& cs = Case();
            for (int i = 0; i < (int)cs.first.size(); i++) {
                if (i > 0) str.append(",");
                str.append(cs.first[i].ToString());
            }
            str.append("(TYPE=").append(std::to_string(cs.second)).append(")");
            return str;
        }
    case LABEL:
        {
            auto& prop = Property();
            if (pretty) str.append("expr::label = {");
            str.append(prop.first.ToString()).append(":").append(prop.second);
            if (pretty) str.append("}");
            return str;
        }
    case PROPERTY:
        {
            auto& prop = Property();
            if (pretty) str.append("expr::prop = {");
            str.append(prop.first.ToString()).append(".").append(prop.second);
            if (pretty) str.append("}");
            return str;
        }
    case FILTER:
        return Filter()->ToString();
    case LIST_COMPREHENSION:
        {
            auto& lc = ListComprehension();
            str.append("[").append(lc[0].ToString()).append(" IN ").append(lc[1].ToString());
            if (lc[2].type != NA) str.append(" WHERE ").append(lc[2].ToString());
            str.append(" | ").append(lc[3].ToString()).append("]");
            return str;
        }
    case RELATIONSHIPS_PATTERN:
        return "(expr::relationship_pattern)";
    default:
        return "(expr:NA)";
    }
}

}  // namespace parser
