
#include <iostream>
#include <string>

#include "cypher/execution_plan/plan_cache/plan_cache_param.h"
#include "./antlr4-runtime.h"
#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor.h"
#include "cypher/parser/cypher_error_listener.h"
#include "cypher/execution_plan/execution_plan.h"
#include "cypher/parser/expression.h"
#include "cypher/execution_plan/runtime_context.h"
#include "server/bolt_server.h"
#include "server/bolt_session.h"
#include "db/galaxy.h"

int main() {
    // std::cout<<cypher::fastQueryParam("match (n:Person) where n.id=101 return n;")<<std::endl;
    std::string input;
    // 构建 test suit RTContext
    cypher::RTContext ctx;
    std::string param_query = fastQueryParam(&ctx, "match (n:Person) where n.id=101 return n;");
    std::cout<<param_query<<std::endl;
    std::cout<<ctx.query_params_[0].ToString()<<std::endl;

    // 根据参数化的查询进行语法解析
    antlr4::ANTLRInputStream input_stream(param_query);
    parser::LcypherLexer lexer(&input_stream);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::LcypherParser lparser(&tokens);
    lparser.addErrorListener(&parser::CypherErrorListener::INSTANCE);
    parser::CypherBaseVisitor visitor(&ctx, lparser.oC_Cypher());
    for (const auto &sql_query: visitor.GetQuery()) {
        std::cout<< sql_query.ToString();
    }

    std::shared_ptr<cypher::ExecutionPlan> plan;
    plan = std::make_shared<cypher::ExecutionPlan>();
    plan->PreValidate(&ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
    plan->Build(visitor.GetQuery(), visitor.CommandType(), &ctx);
    plan->Execute(&ctx);
    // plan->Validate(&ctx);
    
    // while (true) {
    //     std::cout << "-> ";
    //     std::getline(std::cin, input);
    //     if (input == "exit") {
    //         break;
    //     }
    //     std::vector<parser::Expression> params;
    //     std::string res = cypher::fastQueryParam(input, params);

    //     // 输出结果
    //     std::cout << "Parameterizd result: " << res << std::endl;
    //     std::cout << "Params: [";
    //     for (size_t i = 0; i < params.size(); i++) {
    //         std::cout << params[i].ToString();
    //         if (i!= params.size() - 1) {
    //             std::cout << ", ";
    //         }
    //     }
    //     std::cout << "];" << std::endl;
    // }
    return 0;
}