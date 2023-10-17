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

#include <boost/any.hpp>
#include "cypher/cypher_exception.h"
#include "filter/filter.h"
#include "cypher/parser/data_typedef.h"

namespace parser {

struct Expression {
    typedef std::string EXPR_TYPE_STRING;
    typedef std::string EXPR_TYPE_POINT;
    typedef MAP_MAP_LITERAL EXPR_TYPE_MAP;
    typedef std::vector<Expression> EXPR_TYPE_LIST;
    typedef lgraph::Filter EXPR_TYPE_FILTER;
    typedef std::pair<Expression, std::string> EXPR_TYPE_PROPERTY;
    typedef std::pair<Expression, VEC_STR> EXPR_TYPE_LABEL;
    /* the last integer indicates type:
     * 0: CASE CaseAlternatives END
     * 1: CASE CaseAlternatives ELES e END
     * 2: CASE e CaseAlternatives END
     * 3: CASE e CaseAlternatives ELES e END
     * list layout: [case_alternative]+ [head] [tail]  */
    typedef std::pair<EXPR_TYPE_LIST, int> EXPR_TYPE_CASE;
    // var, (IN)expr, where, (|)expr
    typedef std::vector<Expression> EXPR_LIST_COMPREHENSION;
    typedef TUP_PATTERN_ELEMENT EXPR_RELATIONSHIPS_PATTERN;

    Expression() : type(NA), data(0) {}

    // NOTE: use shared_ptr for non-primitive types, so there is no need of deep copy & free
    enum DataType {
        INT,        // integer literal
        DOUBLE,     // double literal
        BOOL,       // boolean literal
        STRING,     // string literal
        MAP,        // map literal
        VARIABLE,   // variable (also a string)
        PARAMETER,  // parameter (also a string)
        LIST,       // list of expressions
        FILTER,     // lgraph::Filter
        PROPERTY,   // property, e.g. n.name
        LABEL,      // node label, e.g. n:Person
        FUNCTION,   // function invocation (vector of expr)
        CASE,       // case expression
        MATH,       // math expression, e.g. 1+(6/5-2)
        LIST_COMPREHENSION,
        RELATIONSHIPS_PATTERN,
        NULL_,  // null
        NA      // not applicable
    } type;

    boost::any data;

    bool IsLiteral() const { return type <= STRING; }

    int64_t Int() const {
#if __APPLE__
        CYPHER_THROW_ASSERT(data.type() == typeid(long long) ||  // NOLINT
                            data.type() == typeid(long));        // NOLINT
        if (data.type() == typeid(long long)) {                  // NOLINT
            return boost::any_cast<long long>(data);             // NOLINT
        } else {
            return boost::any_cast<long>(data);                  // NOLINT
        }
#else
        CYPHER_THROW_ASSERT(data.type() == typeid(int64_t));
        return boost::any_cast<int64_t>(data);
#endif
    }

    double Double() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(double));
        return boost::any_cast<double>(data);
    }

    bool Bool() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(bool));
        return boost::any_cast<bool>(data);
    }

    const EXPR_TYPE_STRING& String() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_STRING>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_STRING>>(data);
    }

    const EXPR_TYPE_MAP& Map() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_MAP>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_MAP>>(data);
    }

    const EXPR_TYPE_LIST& List() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_LIST>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_LIST>>(data);
    }

    const EXPR_TYPE_CASE& Case() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_CASE>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_CASE>>(data);
    }

    const EXPR_LIST_COMPREHENSION& ListComprehension() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_LIST_COMPREHENSION>));
        return *boost::any_cast<std::shared_ptr<EXPR_LIST_COMPREHENSION>>(data);
    }

    const EXPR_RELATIONSHIPS_PATTERN& RelationshipsPattern() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_RELATIONSHIPS_PATTERN>));
        return *boost::any_cast<std::shared_ptr<EXPR_RELATIONSHIPS_PATTERN>>(data);
    }

    const EXPR_TYPE_PROPERTY& Property() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_PROPERTY>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_PROPERTY>>(data);
    }

    const EXPR_TYPE_LABEL& Labels() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_LABEL>));
        return *boost::any_cast<std::shared_ptr<EXPR_TYPE_LABEL>>(data);
    }

    std::shared_ptr<EXPR_TYPE_FILTER> Filter() const {
        CYPHER_THROW_ASSERT(data.type() == typeid(std::shared_ptr<EXPR_TYPE_FILTER>));
        return boost::any_cast<std::shared_ptr<EXPR_TYPE_FILTER>>(data);
    }

    void Label2Filter() {
        CYPHER_THROW_ASSERT(type == LABEL);
        auto& l = Labels();
        std::shared_ptr<lgraph::Filter> f = std::make_shared<lgraph::LabelFilter>(
            l.first.String(), std::set<std::string>{l.second.begin(), l.second.end()});
        data = f;
        type = FILTER;
    }

    bool operator==(const Expression& rhs) const {
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

    bool ContainAlias(const std::unordered_set<std::string>& alias) const {
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

    std::string ToString(bool pretty = true) const {
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
                str.append(list[0].String()).append("(");
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
};

}  // namespace parser
