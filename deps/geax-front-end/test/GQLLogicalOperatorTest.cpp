/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>
#include <cstdint>
#include <deque>
#include <memory>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax/logical/Logical.h"
#include "geax/logical/LogicalOperator.h"

class GQLLogicalOperatorTest : public ::testing::Test {};

TEST_F(GQLLogicalOperatorTest, demo) {
    // -----------------------------------
    // Query:
    //   MATCH (n:person)-[r]->(m)
    //   WHERE n.age > 20 AND m.name = 'k'
    //   RETURN r.timestamp
    // -----------------------------------
    // Expr: n.age > 20
    geax::frontend::VInt age1;
    age1.setVal(20);
    geax::frontend::Ref n;
    n.setName("n");
    geax::frontend::GetField nGetAge;
    nGetAge.setExpr(&n);
    nGetAge.setFieldName("age");
    geax::frontend::BGreaterThan nAgeGreaterThan20;
    nAgeGreaterThan20.setLeft(&nGetAge);
    nAgeGreaterThan20.setRight(&age1);
    // Expr: m.name = 'k'
    geax::frontend::VString name;
    name.setVal("k");
    geax::frontend::Ref m;
    m.setName("m");
    geax::frontend::GetField mGetName;
    mGetName.setExpr(&m);
    mGetName.setFieldName("name");
    geax::frontend::BEqual mNameEqualK;
    mNameEqualK.setLeft(&mGetName);
    mNameEqualK.setRight(&name);
    // Expr: r.timestamp
    geax::frontend::Ref r;
    r.setName("r");
    geax::frontend::GetField rGetTimestamp;
    rGetTimestamp.setExpr(&r);
    rGetTimestamp.setFieldName("timestamp");
    // Logical Plan
    geax::frontend::SingleLabel person;
    person.setLabel("person");
    auto nodeByLabelsScan =
        std::make_shared<geax::logical::LogicalNodeByLabelsScan>(
            "n", (geax::frontend::LabelTree*)&person);
    auto filter1 = std::make_shared<geax::logical::LogicalFilter>(
        std::vector<geax::frontend::Expr*>{&nAgeGreaterThan20});
    auto filter2 = std::make_shared<geax::logical::LogicalFilter>(
        std::vector<geax::frontend::Expr*>{&mNameEqualK});
    auto expandAll = std::make_shared<geax::logical::LogicalExpandAll>(
        "n", "r", "m", geax::frontend::EdgeDirection::kPointRight, nullptr,
        std::vector<geax::frontend::Expr*>{});
    auto projection = std::make_shared<geax::logical::LogicalProjection>(
        std::vector<std::tuple<geax::frontend::Expr*, std::string>>{
            {(geax::frontend::Expr*)&rGetTimestamp,
             std::string("r.timestamp")}});
    auto produceResults =
        std::make_shared<geax::logical::LogicalProduceResults>();
    produceResults->addChild(projection);
    projection->addChild(filter2);
    filter2->addChild(expandAll);
    expandAll->addChild(filter1);
    filter1->addChild(nodeByLabelsScan);
    // dump
    std::function<void(std::shared_ptr<geax::logical::LogicalOperator>,
                       int64_t)>
        dumpOp = [&](std::shared_ptr<geax::logical::LogicalOperator> op,
                     int64_t indent) {
            std::string s;
            s.append(std::string(indent, ' ')).append(op->toString());
            std::cout << s << std::endl;
            for (auto child : op->children()) {
                dumpOp(child, indent + 2);
            }
        };
    dumpOp(produceResults, 0);
    // check parent
    std::function<void(std::shared_ptr<geax::logical::LogicalOperator>)>
        checkParent = [&](std::shared_ptr<geax::logical::LogicalOperator> op) {
            for (auto child : op->children()) {
                EXPECT_EQ(
                    !child->parent().expired() && child->parent().lock() == op,
                    true);
                checkParent(child);
            }
        };
    checkParent(produceResults);
}
