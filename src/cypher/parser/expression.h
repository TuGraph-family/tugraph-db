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
#include "cypher/parser/data_typedef.h"

namespace lgraph {
class Filter;
}

namespace parser {
struct Expression {
    typedef std::string EXPR_TYPE_STRING;
    typedef MAP_MAP_LITERAL EXPR_TYPE_MAP;
    typedef std::vector<Expression> EXPR_TYPE_LIST;
    typedef std::pair<Expression, std::string> EXPR_TYPE_PROPERTY;
    typedef std::pair<Expression, VEC_STR> EXPR_TYPE_LABEL;
    typedef lgraph::Filter EXPR_TYPE_FILTER;
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

    std::shared_ptr<EXPR_TYPE_FILTER> Filter() const;
    void Label2Filter();
    bool operator==(const Expression& rhs) const;
    bool ContainAlias(const std::unordered_set<std::string>& alias) const;
    std::string ToString(bool pretty = true) const;
};

}  // namespace parser
