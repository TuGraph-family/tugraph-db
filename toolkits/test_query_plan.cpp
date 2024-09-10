
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

std::string eval(std::string query);

int main(int argc, char* argv[]) {
    // lgraph::import_v3::Importer::Config config;
    // config.config_file =
    //     "../../test/resource/data/mini_finbench/mini_finbench.json";
    // config.db_dir = "plan_cache_db";
    // config.delete_if_exists = true;
    // config.graph = "default";
    // config.delimiter = "|";
    // lgraph::import_v3::Importer importer(config);
    // importer.DoImportOffline();
    
    // 读取命令行第一个参数为string
    // if (argc!= 2) {
    //     std::cout << "Usage: " << argv[0] << " <query>" << std::endl;
    //     return 1;
    // }
    // std::string query = argv[1];
    std::string query = "MATCH (a) WHERE a.name IN ['Rachel Kempson'] RETURN a";
    // std::string path = ""
    lgraph::Galaxy::Config gconf;
    gconf.dir = "./testdb";
    lgraph::Galaxy galaxy(gconf, true, nullptr);
    cypher::RTContext ctx(nullptr, &galaxy, "admin", "default");
    std::string input;
    // 构建 test suit RTContext
    std::string param_query = fastQueryParam(&ctx, query);
    std::cout<<param_query<<std::endl;
    for (size_t i = 0; i < ctx.query_params_.size(); i++) {
        std::cout<<ctx.query_params_[i].ToString()<<", ";
    }
    std::cout<<std::endl;

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

    std::string cache_res = ctx.result_->Dump(false);
    // std::string eval_res = eval(query);

    std::cout << "cache_res: " << cache_res << std::endl;
    // std::cout << "eval_res: " << eval_res << std::endl;
    return 0;
}

std::string eval(std::string query) {
    lgraph::Galaxy::Config gconf;
    gconf.dir = "./testdb";
    lgraph::Galaxy galaxy(gconf, true, nullptr);
    cypher::RTContext ctx(nullptr, &galaxy, "admin", "default");
    antlr4::ANTLRInputStream input_stream(query);
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
    return ctx.result_->Dump(false);
}