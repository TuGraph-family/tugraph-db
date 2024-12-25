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
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>

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
#include "builder/static_var.h"
using builder::dyn_var;
using builder::static_var;
using cypher::compilation::CFieldData;
using cypher::compilation::CScalarData;
using cypher::compilation::CRecord;
using cypher::compilation::CEntry;

#include "gtest/gtest.h"

#include "core/value.h"
#include "./ut_utils.h"

std::string execute(const std::string& command) {
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "popen() failed!" << std::endl;
        return "";
    }
    char buf[128];
    while (fgets(buf, sizeof(buf), pipe) != nullptr) {
        result += buf;
    }
    pclose(pipe);
    return result;
}

std::string execute_func(std::string &func_body) {
    const std::string file_name = "test_add.cpp";
    const std::string output_name = "test_add";
    std::ofstream out_file(file_name);
    if (!out_file) {
        std::cerr << "Failed to open file for writing!" << std::endl;
        return "";
    }
    out_file << func_body;
    out_file.close();
    // define and execute compiler commands
    std::string compile_cmd = "g++ " + file_name + " -o " + output_name;
    int compile_res = system(compile_cmd.c_str());
    if (compile_res != 0) {
        std::cerr << "Compilation failed!" << std::endl;
        return "";
    }
    // define and execute command
    std::string output = execute("./a");
    // delete files
    if (std::remove(file_name.c_str()) && std::remove(output_name.c_str())) {
        std::cerr << "Failed to delete files: " << file_name
                  << ", " << output_name << std::endl;
    }
    return output;
}
class TestQueryCompilation : public TuGraphTest {};

dyn_var<int64_t> add(void) {
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

    cypher::compilation::ExprEvaluator evaluator(&add, &sym_tab);
    cypher::RTContext ctx;
    return evaluator.Evaluate(&ctx, &record).constant_.scalar.Int64();
}

TEST_F(TestQueryCompilation, Add) {
    builder::builder_context context;
    auto ast = context.extract_function_ast(add, "add");
    std::ostringstream oss;
    oss << "#include <iostream>\n";
    block::c_code_generator::generate_code(ast, oss, 0);
    oss << "int main() {\n  std::cout << add();\n  return 0;\n}";
    std::string body = oss.str();
    std::cout <<"Generated code: \n" << body << std::endl;
    std::string res = execute_func(body);
    ASSERT_EQ(res, "20");
}
