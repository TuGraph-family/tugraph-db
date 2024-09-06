
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

std::string eval();

int main() {
    // lgraph::import_v3::Importer::Config config;
    // config.config_file =
    //     "../../test/resource/data/mini_finbench/mini_finbench.json";
    // config.db_dir = "plan_cache_db";
    // config.delete_if_exists = true;
    // config.graph = "default";
    // config.delimiter = "|";
    // lgraph::import_v3::Importer importer(config);
    // importer.DoImportOffline();

    lgraph::Galaxy::Config gconf;
    gconf.dir = "./plan_cache_db";
    lgraph::Galaxy galaxy(gconf, true, nullptr);
    cypher::RTContext ctx(nullptr, &galaxy, "admin", "default");
    std::string input;
    // 构建 test suit RTContext
    std::string param_query = fastQueryParam(&ctx, "MATCH (a:Person) WHERE a.birthyear < 1960 OR a.birthyear >= 1970 RETURN a.name;");
    std::cout<<param_query<<std::endl;
    std::cout<<ctx.query_params_[0].ToString()<<std::endl;
    std::cout<<ctx.query_params_[1].ToString()<<std::endl;

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

    std::string cache_res = ctx.result_->Dump();
    std::string eval_res = eval();

    assert(cache_res == eval());
    return 0;
}

std::string eval() {
    lgraph::Galaxy::Config gconf;
    gconf.dir = "./plan_cache_db";
    lgraph::Galaxy galaxy(gconf, true, nullptr);
    cypher::RTContext ctx(nullptr, &galaxy, "admin", "default");
    antlr4::ANTLRInputStream input_stream("MATCH (a:Person) WHERE a.birthyear < 1960 OR a.birthyear >= 1970 RETURN a.name;");
    parser::LcypherLexer lexer(&input_stream);
    antlr4::CommonTokenStream tokens(&lexer);
    parser::LcypherParser lparser(&tokens);
    lparser.addErrorListener(&parser::CypherErrorListener::INSTANCE);
    parser::CypherBaseVisitor visitor(&ctx, lparser.oC_Cypher());
    std::shared_ptr<cypher::ExecutionPlan> plan;
    plan = std::make_shared<cypher::ExecutionPlan>();
    plan->PreValidate(&ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
    plan->Build(visitor.GetQuery(), visitor.CommandType(), &ctx);
    plan->Execute(&ctx);
    return ctx.result_->Dump();
}