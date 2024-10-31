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

#include <variant>
#include "cypher/experimental/data_type/field_data.h"
#include "cypher/experimental/data_type/record.h"
#include "cypher/experimental/expressions/cexpr.h"
#include "cypher/parser/symbol_table.h"
#include "cypher/execution_plan/runtime_context.h"
#include "geax-front-end/ast/Ast.h"
#include "blocks/c_code_generator.h"
#include "builder/builder.h"
#include "builder/builder_context.h"
#include "builder/dyn_var.h"
using namespace cypher::compilation;
using builder::static_var;
using builder::dyn_var;

dyn_var<int64_t> bar(void) {
    std::variant<static_var<int64_t>, static_var<int>> a;
    std::variant<dyn_var<int64_t>, dyn_var<int>> b;
    a = (std::variant<static_var<int64_t>, static_var<int>>)static_var<int64_t>(10);
    b = dyn_var<int64_t>(10);
    auto res = std::get<static_var<int64_t>>(a) + std::get<dyn_var<int64_t>>(b);
    return res;
}

dyn_var<int64_t> foo(void) {
    cypher::SymbolTable sym_tab;

    CFieldData a(std::move(CScalarData(10)));
    geax::frontend::Ref ref1;
    ref1.setName(std::string("a"));
    sym_tab.symbols.emplace("a",
        cypher::SymbolNode(0, cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL));

    CFieldData b(static_var<int64_t>(10));
    geax::frontend::Ref ref2;
    ref2.setName(std::string("b"));
    sym_tab.symbols.emplace("b",
        cypher::SymbolNode(1, cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL));

    geax::frontend::BAdd add;
    add.setLeft((geax::frontend::Expr*)&ref1);
    add.setRight((geax::frontend::Expr*)&ref2);
    CRecord record;
    record.values.push_back(CEntry(a));
    record.values.push_back(CEntry(b));

    ExprEvaluator evaluator(&add, &sym_tab);
    cypher::RTContext ctx;
    return evaluator.Evaluate(&ctx, &record).constant_.scalar.Int64();
}

int main() {
    builder::builder_context context;
    std::cout << "#include <iostream>" << std::endl;
    block::c_code_generator::generate_code(context.extract_function_ast(foo, "foo"), std::cout, 0);
    std::cout << "int main() {\n  std::cout << foo() << std::endl;\n  return 0;\n}";
    return 0;
}
