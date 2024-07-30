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

#include <db/galaxy.h>
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "fma-common/utils.h"
#include "./ut_utils.h"
#include "gtest/gtest.h"
#include "core/data_type.h"

/* Make sure include graph_factory.h BEFORE antlr4-runtime.h. Otherwise causing the following error:
 * ‘EOF’ was not declared in this scope.
 * For the former (include/butil) uses macro EOF, which is undefined in antlr4. */
#include "./graph_factory.h"
#include "./antlr4-runtime.h"

#include "cypher/execution_plan/execution_plan.h"
#include "cypher/execution_plan/scheduler.h"
#include "lgraph/lgraph_utils.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor.h"
#include "cypher/parser/cypher_error_listener.h"

using namespace parser;
using namespace antlr4;

static const cypher::PARAM_TAB g_param_tab = {
    {"$name", cypher::FieldData(lgraph::FieldData("Lindsay Lohan"))},
    {"$plugin_type", cypher::FieldData(lgraph::FieldData("CPP"))},
    {"$new_name", cypher::FieldData(lgraph::FieldData("new name"))},
    {"$personId", cypher::FieldData(lgraph::FieldData(1))},
    {"$personIds", cypher::FieldData(std::vector<lgraph::FieldData>{
                       lgraph::FieldData("Liam Neeson"), lgraph::FieldData("Dennis Quaid"),
                       lgraph::FieldData("Roy Redgrave")})},
};

int test_file_script(const std::string &file, cypher::RTContext *ctx) {
    ANTLRFileStream input;
    input.loadFromFile(file);
    // ANTLRFileStream input;
    // input.loadFromFile(file);
    LcypherLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    LcypherParser parser(&tokens);
    /* We can set ErrorHandler here.
     * setErrorHandler(std::make_shared<BailErrorStrategy>());
     * add customized ErrorListener  */
    parser.addErrorListener(&CypherErrorListener::INSTANCE);
    CypherBaseVisitor visitor(ctx, parser.oC_Cypher());
    auto stmt = visitor.GetQuery();
    for (auto &s : stmt) {
        UT_LOG() << s.ToString();
        for (auto &p : s.parts) p.symbol_table.DumpTable();
    }
    cypher::ExecutionPlan execution_plan;
    execution_plan.PreValidate(ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
    execution_plan.Build(stmt, visitor.CommandType(), ctx);
    execution_plan.Validate(ctx);
    execution_plan.DumpGraph();
    execution_plan.DumpPlan(0, false);
    execution_plan.Execute(ctx);
    UT_LOG() << "Result:\n" << ctx->result_->Dump(false);
    return 0;
}

int test_interactive(cypher::RTContext *ctx) {
    auto visitor = CypherBaseVisitor();
    std::string script;
    UT_LOG() << "LightningGraph Cypher Interpreter";
    while (true) {
        try {
            UT_LOG() << ">> ";
            getline(std::cin, script);
            if (script == "exit") break;
            ANTLRInputStream input(script);
            LcypherLexer lexer(&input);
            CommonTokenStream tokens(&lexer);
            LcypherParser parser(&tokens);
            parser.addErrorListener(&CypherErrorListener::INSTANCE);
            visitor.visit(parser.oC_Cypher());
            cypher::ExecutionPlan execution_plan;
            execution_plan.PreValidate(ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
            execution_plan.Build(visitor.GetQuery(), visitor.CommandType(), ctx);
            execution_plan.Validate(ctx);
            execution_plan.Execute(ctx);
            UT_LOG() << "Result:\n" << ctx->result_->Dump(false);
        } catch (std::exception &e) {
            UT_LOG() << e.what();
            UT_EXPECT_TRUE(false);
        }
    }
    return 0;
}

void eval_scripts_check(cypher::RTContext *ctx, const std::vector<std::string> &scripts,
                        const std::vector<int> &check) {
    for (int i = 0; i < (int)scripts.size(); i++) {
        auto s = scripts[i];
        UT_LOG() << i << "th:" << s;
        ANTLRInputStream input(s);
        LcypherLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        LcypherParser parser(&tokens);
        parser.addErrorListener(&CypherErrorListener::INSTANCE);
        CypherBaseVisitor visitor(ctx, parser.oC_Cypher());
        cypher::ExecutionPlan execution_plan;
        execution_plan.PreValidate(ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
        execution_plan.Build(visitor.GetQuery(), visitor.CommandType(), ctx);
        execution_plan.Validate(ctx);
        execution_plan.DumpGraph();
        execution_plan.DumpPlan(0, false);
        execution_plan.Execute(ctx);
        UT_LOG() << "Result:\n" << ctx->result_->Dump(false);
        if (scripts.size() == check.size()) UT_EXPECT_EQ(ctx->result_->Size(), check[i]);
    }
}

void eval_scripts(cypher::RTContext *ctx, const std::vector<std::string> &scripts) {
    eval_scripts_check(ctx, scripts, std::vector<int>{});
}

void eval_script(cypher::RTContext *ctx, const std::string &script) {
    eval_scripts(ctx, std::vector<std::string>{script});
}

void expected_exception_any(cypher::RTContext *ctx, const std::string &script) {
    UT_EXPECT_ANY_THROW(eval_script(ctx, script));
}

void expected_exception_undefined_var(cypher::RTContext *ctx, const std::string &script) {
    try {
        eval_script(ctx, script);
        UT_EXPECT_TRUE(false);
    } catch (lgraph_api::LgraphException& e) {
        UT_EXPECT_EQ(e.code(), lgraph_api::ErrorCode::InputError);
        std::string exception_msg(e.msg());
        if (exception_msg.find("Variable") == 0 &&
            exception_msg.find("not defined") != std::string::npos) {
            UT_LOG() << "Expected exception(undefined variable): " << e.what();
        } else {
            throw e;
        }
    }
}

int test_find(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person {name:'Vanessa Redgrave'}) RETURN n", 1},
        {"MATCH (m:Film {title:'The Parent Trap'}) RETURN m.title,m", 1},
        {"MATCH (people:Person) RETURN people.name LIMIT 7", 7},
        {"MATCH (people:Person) RETURN people.name SKIP 7", 6},
        {"MATCH (people:Person) RETURN people.name SKIP 3 LIMIT 4", 4},
        {"MATCH (post60s:Person) WHERE post60s.birthyear >= 1960 AND post60s.birthyear < 1970 "
         "RETURN post60s.name",
         2},
        {"MATCH (a:Person) WHERE a.birthyear < 1960 OR a.birthyear >= 1970 RETURN a.name", 11},
        {"MATCH (a:Person) WHERE a.birthyear >= 1960 XOR id(a) > 10 RETURN a,a.birthyear", 4},
        // weak index lookup
        {"MATCH (n {name:'Vanessa Redgrave'}) RETURN n", 1},
        {"MATCH (n:Person)-[r:BORN_IN]->() WHERE abs(r.weight-20.21)<0.00001 RETURN n,r,r.weight",
         1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

void test_invalid_schema(cypher::RTContext *ctx) {
    std::string cypher;
    cypher = "MATCH (n:Person_x {name:'Vanessa Redgrave'})-[:ACTED_IN]->(m) RETURN n,m.title";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "MATCH (n:Person {name_x:'Vanessa Redgrave'})-[:ACTED_IN]->(m) RETURN n,m.title";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "MATCH (n:Person {name:'Vanessa Redgrave'})-[:ACTED_IN_x]->(m) RETURN n,m.title";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "MATCH (n)<-[relatedTo]-(vanessa:Person_x {name:'Vanessa'}) RETURN n,relatedTo";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "MATCH (n)<-[relatedTo]-(vanessa:Person {name_x:'Vanessa'}) RETURN n,relatedTo";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "CREATE (:City {name:'Shanghai'}), (:City_x {name:'Zhongshan'})";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "CREATE (:City {name_x:'Shanghai'}), (:City {name:'Zhongshan'})";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
    cypher = "MATCH (c {name:'Houston'}) WITH c "
        "MATCH (p:Person {name:'Liam Neeson'}) CREATE (c)-[:HAS_CHILD_x]->(p)";
    UT_EXPECT_THROW_MSG(eval_script(ctx, cypher), "No such")
}

int test_query(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[:ACTED_IN]->(m) RETURN n,m.title", 1},
        {"MATCH (:Person {name:'Vanessa Redgrave'})-[:ACTED_IN]->(movie) return movie.title", 1},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[relatedTo]-(b) RETURN b,relatedTo", 5},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD]-(b) RETURN b.name", 3},
        {"MATCH (m:Film {title:'Batman Begins'})<-[:DIRECTED]-(directors) RETURN directors.name",
         1},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[]-(neighbors) RETURN neighbors", 5},
        {"MATCH (n:Person {name:'Lindsay Lohan'})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors) RETURN "
         "coActors.name",
         2},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[]->()<-[]-(m) RETURN m", 4},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[]->()<-[]-(m) RETURN DISTINCT m", 4},
        {"MATCH (na)-[]->(nb)-[]->(nc) RETURN na,nb,nc", 27},
        {"MATCH (na:Person)-[:HAS_CHILD]->(nb)-[:MARRIED]->(nc) RETURN na,nb,nc", 2},
        {"MATCH (m:Film {title:'Camelot'})<-[r:ACTED_IN]-(n) RETURN n.name,r.charactername", 2},
        {"MATCH (n)-[relatedTo]->(vanessa:Person {name:'Vanessa Redgrave'}) RETURN n,relatedTo", 2},
        {"MATCH (n)<-[relatedTo]-(vanessa:Person {name:'Vanessa Redgrave'}) RETURN n,relatedTo", 3},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[]->(b:Person) RETURN b.name", 1},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b:Person) RETURN b.name", 3},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person RETURN b.name", 3},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person AND b.birthyear >= 1910 "
         "RETURN b.name",
         2},
        {"MATCH (a:Person {name:'Vanessa Redgrave'})-[]-(b) WHERE b:Person OR b:City RETURN b.name",
         4},
        {"MATCH (n:Person {name:'Lindsay "
         "Lohan'})-[:ACTED_IN]->(m)<-[:ACTED_IN]-(coActors)-[:BORN_IN]->(city) RETURN "
         "coActors.name,city.name",
         2},
        /* test filter optimization */
        {"MATCH (a)-->(b)-->(c)<--(d) USING JOIN ON c WHERE a.uid > 1 AND d.uid > 2 AND b.uid < 3 "
         "AND c.uid < 4 RETURN d",
         0},
        {"MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > 1 AND d.uid > 2 AND b.uid < 3 AND c.uid < 4 "
         "RETURN d",
         0},
        {"MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > d.uid AND b.uid < c.uid RETURN d", 0},
        {"MATCH (a)-->(b)-->(c)<--(d) WHERE a.uid > d.uid AND b.uid < c.uid AND a.uid > b.uid "
         "RETURN d",
         0},
        /* test edge filter pushdown optimization */
        {"MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 20.18 RETURN m", 2},
        {"MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 20.18 AND m.name <> 'Houston' RETURN "
         "m",
         1},
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = a.title RETURN n",
         0},  // issue: #168 #169
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE a.role = \"Iron Man\" RETURN n",
         0},  // issue: #168 #169
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = \"Vanessa Redgrave\" RETURN n",
         1},  // issue: #168 #169
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE m.title = \"Camelot\" RETURN n",
         2},  // issue: #168 #169
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name = a.title AND m.title = \"Camelot\" "
         "RETURN n",
         0},  // issue: #168 #169
        {"MATCH (n:Person)-[a:ACTED_IN]->(m) WHERE n.name =  \"Vanessa Redgrave\" AND m.title = "
         "a.title RETURN n",
         0},  // issue: #168 #169
        {"MATCH (n:Person)-[b:BORN_IN]->(m) WHERE b.weight < 19.2 OR b.weight > 20.6 RETURN m",
         2},  // issue: #190
        {"MATCH (n:Person)-[b:BORN_IN]->(m) WHERE (b.weight + b.weight) "
         "< 38.4 OR b.weight > 20.6 RETURN m",
         2},
        {"MATCH (a)-[e]->(b) WHERE a.name='Liam Neeson' and b.title<>'' and "
         "(e.charactername='Henri Ducard' or e.relation = '') RETURN a,e,b",
         1},  // issue: #190
        {"MATCH (a) WHERE a.name IN ['Dennis Quaid', 'Christopher Nolan'] WITH a "
         "MATCH (b) WHERE b.name IN ['London'] RETURN a, b",
         2},
        {"MATCH (a) WHERE a.name IN ['Dennis Quaid', 'Christopher Nolan'] WITH a "
         "MATCH (b) WHERE b.name IN ['London', 'Beijing', 'Houston'] RETURN a, b",
         4},  // issue #305
        {"MATCH (n:Person) WHERE n.name = 'Vanessa Redgrave' "
         "OR NOT n.name <> 'Dennis Quaid' RETURN n.name",
         2},  // issue #332
        /* test multi-types relp */
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[:BORN_IN|ACTED_IN]->(m) RETURN m", 2},
        {"MATCH (n:Person {name:'Michael Redgrave'})<-[:MARRIED|HAS_CHILD]-(m) RETURN m", 2},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[:BORN_IN|HAS_CHILD]-(m) RETURN m", 4},
        /* compare between 2 arithmetic expressions */
        {"MATCH (n:Person {name:'Vanessa Redgrave'})--(m:Person) WHERE m.birthyear > n.birthyear "
         "RETURN m",
         1},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})--(m:Person) WHERE n.birthyear > m.birthyear "
         "RETURN m",
         2},
        /* undirected relp */
        {"MATCH (n:Person {name:'Natasha Richardson'})-[:HAS_CHILD]->(m) RETURN m", 0},
        {"MATCH (n:Person {name:'Natasha Richardson'})<-[:HAS_CHILD]-(m) RETURN m", 1},
        {"MATCH (n:Person {name:'Natasha Richardson'})-[:HAS_CHILD]-(m) RETURN m", 1},
        {"match (a)-->(b)-->(c)<--(d) where not ((not (a.birthyear>d.birthyear and "
         "b.birthyear<c.birthyear)) and (not a.birthyear>b.birthyear)) return d",
         5},

        /* multiple match pattern */
        {"MATCH (n:Person{name:'Vanessa Redgrave'}),(m:Person{name: 'Michael Redgrave'}) WHERE "
         "n.birthyear > 1960 and m.birthyear < 2000 RETURN n.name LIMIT 1",
         0},  // #issue 192
        /* test parallel traversal optimization */
        {"MATCH (n:Person) RETURN count(n)", 1},
        {"MATCH (n:Person) WHERE n.birthyear > 1900 AND n.birthyear < 2000 RETURN count(n)", 1},
        {"MATCH (n:Person) RETURN n.birthyear, count(n)", 13},
        {"MATCH (f:Film)<-[:ACTED_IN]-(p:Person)-[:BORN_IN]->(c:City) "
         "RETURN c.name, count(f) AS sum ORDER BY sum DESC",
         3},
        /* test schema rewrite optimization */
        {"MATCH p=(n1)-[r1]->(n2)-[r2]->(m:Person) return count(p)", 1},
        {"MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City),p2=(n3)-[r3]->(m2:Film) return count(p1)", 1},
        {"MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City) with count(p1) as cp match "
         "p1=(n1)-[r1]->(m1:Film) return count(p1)",
         1}};
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_hint(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (rachel:Person {name:'Rachel "
         "Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person "
         "{name:'Richard Harris'})\n"
         "RETURN family.name",
         1},  // no plan hint
        {"MATCH (rachel:Person {name:'Rachel "
         "Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person "
         "{name:'Richard Harris'})\n"
         "USING JOIN ON film\n"
         "RETURN family.name",
         1},
        {"MATCH (rachel:Person {name:'Rachel "
         "Kempson'})-[]->(family:Person)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(richard:Person "
         "{name:'Richard Harris'})\n"
         "USING JOIN ON family\n"
         "RETURN family.name",
         1},
        {"MATCH (camelot:Film {title:'Camelot'})<-[:ACTED_IN]-(actor)-[]->(x)\n"
         "USING START ON camelot\n"
         "RETURN x",
         3},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_multi_match(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        /* 1 connected component */
        {"MATCH (p)-[:ACTED_IN]->(x), (p)-[:MARRIED]->(y), (p)-[:HAS_CHILD]->(z) RETURN p,x,y,z",
         2},
        {"MATCH (x)<-[:ACTED_IN]-(p)-[:MARRIED]->(y), (p)-[:HAS_CHILD]->(z) RETURN p,x,y,z", 2},
        /* multi connected components */
        {"MATCH (n:Film), (m:City) RETURN n, m", 15},
        {"MATCH (n1:Person {name: \"John Williams\"})-[]->(m1:Film), "
        "(n2: Person {name: \"Michael Redgrave\"})-[]->(m2:Film) "
        "WHERE m1.title = m2.title RETURN m1, m2", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_optional_match(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person {name:'NoOne'}) RETURN n /* no result */", 0},
        {"OPTIONAL MATCH (n:Person {name:'NoOne'}) RETURN n /* null */", 1},
        {"OPTIONAL MATCH (n:City {name:'London'})-[r]->(m) RETURN n.name, r, m /* partial */",
         1},  // different
        {"OPTIONAL MATCH (n:City {name:'London'})-[r]-(m) RETURN n.name, r, m", 3},
        {"MATCH (n:City {name:'London'}) WITH n.name AS city_name "
         "OPTIONAL MATCH (n:Person {name:'NoOne'}) RETURN n.name, city_name",
         1},
        {"MATCH (n:City) WITH n MATCH (n)-->(m) RETURN n,m", 0},
        {"MATCH (n:City) WITH n OPTIONAL MATCH (n)-->(m) RETURN n,m", 3},
        /* TODO: multi match clause
        {"MATCH (n:City {name:'London'}) OPTIONAL MATCH (n)-[r]->(m) RETURN n.name, r, m", 1},
        {"MATCH (n:City {name:'London'}) OPTIONAL MATCH (m:NoLabel) RETURN n.name, m", 1},
        {"MATCH (n:Person {name:'Vanessa Redgrave'}) "
         "OPTIONAL MATCH (n)-[:ACTED_IN]->(m)-[:NoType]->(l) RETURN n, m, l", 1},
         */
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_union(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person)-[:BORN_IN]->(:City {name:'London'}) RETURN n.name\n"
         "UNION\n"
         "MATCH (n:Person)-[:ACTED_IN]->(:Film {title:'The Parent Trap'}) RETURN n.name",
         6},
        {"MATCH (n:Person) RETURN n.name AS name UNION MATCH (m:Film) RETURN m.title AS name", 18},
        {"MATCH (n:Person)-[:BORN_IN]->(:City {name:'London'}) RETURN n.name\n"
         "UNION\n"
         "MATCH (n:Person)-[:ACTED_IN]->(:Film {title:'The Parent Trap'}) RETURN n.age\n"
         "/* ***** EXPECTED EXCEPTION ***** */",
         0},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    UT_EXPECT_ANY_THROW(eval_scripts_check(ctx, scripts, check));
    return 0;
}

int test_function(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person) RETURN properties(n) LIMIT 2", 2}, /* debugging stack chaos */
        {"MATCH p=(n:Person)-[e*..2]->(m) RETURN properties(p) LIMIT 2", 2},
        {"MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[relatedTo]-(n) RETURN "
         "id(vanessa),type(relatedTo),label(n)",
         5},
        {"MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->() RETURN startNode(r),endNode(r)",
         3},
        {"MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->(n) RETURN properties(n)", 3},
        {"MATCH (vanessa:Person {name:'Vanessa Redgrave'})-[r]->(n) RETURN properties(r)", 3},
        {"MATCH (a) WHERE a.name = 'Vanessa Redgrave' RETURN label(a), labels(a)", 1},
        {"MATCH (a) WHERE a.name = 'Vanessa Redgrave' RETURN keys(a)", 1},
        {"MATCH (a:Person {name:'Vanessa Redgrave'}) RETURN a,-2,9.78,'im a string'", 1},
        {"MATCH (a:Person {name:'Vanessa Redgrave'}) RETURN "
         "a,abs(-2),ceil(0.1),floor(0.9),rand(),round(3.141592),sign(-17),sign(0.1)",
         1},
        {"RETURN toInteger(2.0),toInteger(2.3),toInteger('3')", 1},
        {"RETURN toBoolean(true),toBoolean('True')", 1},
        {"RETURN toFloat(2),toFloat(2.3),toFloat('3'),toFloat('2.019')", 1},
        {"RETURN toString(2),toString(2.3),toString(true),toString('haha')", 1},
        {"RETURN size('hello world!') /*12*/", 1},
        {"MATCH (n:Person) WHERE size(n.name) > 15 RETURN n.name,size(n.name)", 4},
        /* list functions */
        {"WITH ['one','two','three'] AS coll RETURN size(coll)", 1},
        {"WITH ['one','two','three'] AS coll RETURN head(coll)", 1},
        {"WITH ['one','two','three'] AS coll RETURN last(coll)", 1},
        {"WITH ['one','two','three'] AS coll UNWIND coll AS x RETURN collect(x)", 1},
        {"WITH ['one','two','three'] AS coll UNWIND coll AS x WITH collect(x) AS reColl RETURN "
         "head(reColl)",
         1},
#if 0  // TODO(someone):
        {"WITH ['one','two','three'] AS coll UNWIND coll AS x RETURN head(collect(x))", 1},
#endif
        /* aggregate functions */
        {"MATCH (n:Person) RETURN sum(n.birthyear) /* result: 25219 */", 1},
        {"MATCH (n:Person) RETURN label(n),sum(n.birthyear) /* result: Person,25219 */", 1},
        {"MATCH (n:Person) RETURN n.name,sum(n.birthyear) /* result: individual */", 13},
        {"MATCH (n:Person) RETURN n.name,label(n),sum(n.birthyear) /* result: individual */", 13},
        {"MATCH (n:Person {name:'Natasha Richardson'})--(m:Person) RETURN m.name,sum(m.birthyear) "
         "/* 1937,3904 */",
         2},
        {"MATCH (n:Person) RETURN count(n) /* result: 13 */", 1},
        {"MATCH (n:Person) RETURN avg(n.birthyear) /* result: 1939.923 */", 1},
        {"MATCH (n:Person) RETURN max(n.birthyear),min(n.birthyear),sum(n.birthyear) /* result: "
         "1986,1873,25219 */",
         1},
        {"OPTIONAL MATCH (n:City {name:'London'})-[r]->() RETURN count(r) /* 0 */", 1},
        {"OPTIONAL MATCH (n:City {name:'London'})-[r]->() RETURN count(*) /* 0 */", 1},
        /* as clause */
        {"MATCH (n:Person) RETURN count(n) AS num_person /* result: 13 */", 1},
        /* coalesce */
        {"match (city:City {name:'New York'}) return id(city) as cityId, coalesce(city.name, "
         "city.cname) as cityName",
         1},
        {"RETURN coalesce(null)", 1},
        {"RETURN coalesce(2021)", 1},
        {"RETURN coalesce(2021, null)", 1},
        {"RETURN coalesce(null, null)", 1},
        {"MATCH (n) RETURN coalesce(n.birthyear, n.name)", 21},
        {"RETURN LENGTH('abc1234')", 1},
        {"RETURN SUBSTRING('abc1234', 4, 4)", 1},
        {"RETURN CONCAT('abc', '12', '34')", 1},
        {"RETURN CONCAT('abc', '12', '34', '56', '78')", 1},
        {"RETURN MASK('123456789', 1, 2)", 1},
        {"RETURN MASK('张三', 2, 2)", 1},
        {"RETURN MASK('123456789', 2, 3, '?')", 1}
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN abs('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN ceil('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN floor('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN round('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN sign('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN toboolean('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN tofloat('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN tointeger('haha')"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN SUBSTRING('1234567', -1, 4)"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN SUBSTRING('1234567', 10, 4)"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN SUBSTRING('1234567', 4, 10)"));
    UT_EXPECT_ANY_THROW(eval_script(ctx, "RETURN CONCAT('abc', 12, 1.0)"));
    return 0;
}

int test_parameter(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person) WHERE n.name = $name RETURN n", 1},
        {"MATCH (n:Person {name:$name}) RETURN n", 1},
        {"CREATE (n:Person {name:$new_name}) RETURN n", 1},
        {"MATCH (n:Person {name:$new_name}) DELETE n", 1},
        {"CALL db.plugin.listPlugin($plugin_type, 'any')", 0}
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_var_len_expand(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person) RETURN COUNT(*)", 1},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]->(n) RETURN n", 5},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]->(n)-[:ACTED_IN]->(m) RETURN "
         "n,m",
         3},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..5]->(n) RETURN n", 5},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..2]->(n) RETURN n", 3},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2..5]->(n) RETURN n", 4},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1..]->(n) RETURN n", 5},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2..]->(n) RETURN n", 4},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..2]->(n) RETURN n", 3},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..3]->(n) RETURN n", 5},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*1]->(n) RETURN n", 1},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*2]->(n) RETURN n", 2},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[*..]->(n) RETURN DISTINCT n,n.name,n.title",
         6},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[*3]->(n) RETURN n", 2},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[*2..]->(n) RETURN n", 7},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[]->()-[*0]->(m) RETURN m", 3},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[]->()-[*0..1]->(m) RETURN DISTINCT m", 5},
        {"MATCH (mic:Person {name:'Michael Redgrave'})-[]->()-[*0..1]->(m) RETURN DISTINCT m", 9},
        /* reversed expand */
        {"MATCH (jem:Person {name:'Jemma Redgrave'})<-[:HAS_CHILD*..]-(a) RETURN a", 4},
        {"MATCH (jem:Person {name:'Jemma Redgrave'})<-[:HAS_CHILD*..]-(a)-[:ACTED_IN*..]->(m) "
         "RETURN a,m",
         1},
        /* test multi-types relp */
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD|MARRIED*..]->(n) RETURN DISTINCT "
         "n.name",
         7},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD|MARRIED*1..2]->(n) RETURN DISTINCT "
         "n.name",
         4},
        {"MATCH (liam:Person {name:'Liam Neeson'})<-[:HAS_CHILD|MARRIED*1..3]-(a) RETURN DISTINCT "
         "a.name",
         5},  // liam self included
        /* bi-directional expand */
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]-(n) RETURN n", 13},
        {"MATCH (roy:Person {name:'Roy Redgrave'})-[:HAS_CHILD*..]-(n) RETURN DISTINCT n", 6},
        {"MATCH (jem:Person {name:'Jemma Redgrave'})-[:HAS_CHILD*..]-(a) RETURN a", 13},
        {"MATCH (jem:Person {name:'Jemma Redgrave'})-[:HAS_CHILD*..]-(a) RETURN DISTINCT a", 6},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*..]-(n) RETURN n", 15},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*..]-(n) RETURN DISTINCT n", 7},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*1..2]-(n) RETURN n", 6},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*2]-(n) RETURN n", 3},
        {"MATCH (van:Person {name:'Vanessa Redgrave'})-[:HAS_CHILD*2..]-(n) RETURN n", 12},
        /* the followings are different */
        {"MATCH (n:Person)-[:BORN_IN*0..]->(m) RETURN n.name,m.name", 19},
        {"MATCH (n:Person)-[:BORN_IN*0..]->(m:City) RETURN n.name,m.name", 6},
        {"MATCH (n:Person)-[:BORN_IN*0..]->(m:Person) RETURN n.name,m.name", 13},
        {"MATCH (n:Film)<-[:ACTED_IN*0..]-(m) RETURN n.title,n,m", 13},
        {"MATCH (n:Film)<-[:ACTED_IN*0..]-(m:Person) RETURN n.title,n,m", 8},
        {"MATCH (n:Film)<-[:ACTED_IN*0..]-(m:Film) RETURN n.title,n,m", 5},
        {"MATCH (n:Film)<-[:ACTED_IN*0..]-(m:City) RETURN n.title,n,m", 0},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_uniqueness(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n1:Person {name:'Liam Neeson'})-->(n2)-->(n3)-->(n4) RETURN n4.title", 1},
        {"MATCH (n1:Person {name:'Liam Neeson'})<--(n2)<--(n3)<--(n4) RETURN n4", 2},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[*3]->(n2) RETURN n2", 1},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[*..]->(n2) RETURN n2", 6},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[*2..3]->(n2) RETURN n2", 4},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED*..]->(n2) RETURN n2", 2},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED|ACTED_IN*..]->(n2) RETURN n2", 5},
        {"MATCH (n1:Person {name:'Liam Neeson'})-[:MARRIED|ACTED_IN*2..]->(n2) RETURN n2", 3},
        {"MATCH (n1:Person {name:'Michael Redgrave'})-[*3]->(n2) RETURN n2", 10},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[*2]-(n2) RETURN n2", 2},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[*4]-(n2) RETURN n2", 3},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[*..]-(n2) RETURN n2", 12},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[*..]-(n2) RETURN DISTINCT n2", 6},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED*..]-(n2) RETURN n2", 2},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*..]-(n2) RETURN n2", 12},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*2..4]-(n2) RETURN n2", 7},
        {"MATCH (n1:Person {name:'Liam Neeson'})<-[:MARRIED|HAS_CHILD*2..4]-(n2) RETURN DISTINCT "
         "n2",
         5},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_func_filter(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n) WHERE id(n) = 6 RETURN n.name", 1},
        {"MATCH (n) WHERE id(n) <> 6 RETURN n", 20},
        {"MATCH ()-[r]->() WHERE type(r) = 'ACTED_IN' RETURN r,type(r)", 8},
        // {"MATCH ()-[r]->() WHERE type(4) = 'ACTED_IN' RETURN r,type(r) /* invalid argument */",
        // 28},
        /* test filter optimization */
        {"MATCH (a)-->(b)-->(c)<--(d) WHERE id(b) <> id(d) AND id(a) > id(d) AND id(b) < id(c) "
         "RETURN a,b,c,d",
         4},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_expression(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN n.birthyear, n.birthyear > 1900, n.birthyear > 2000",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN CASE WHEN n.birthyear < 1950 THEN 1 ELSE 2 END AS type /* 2 */",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN CASE WHEN n.birthyear < 1950 THEN 1 ELSE 2 END AS type1,"
         "CASE WHEN n.birthyear = 1952 THEN 1 ELSE 2 END AS type2 /* 2,1 */",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN CASE n.birthyear WHEN 1950 THEN 1 WHEN 1960 THEN 2 END AS type",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN CASE n.birthyear WHEN 1950 THEN 1 WHEN 1960 THEN 2 ELSE 3 END AS type",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}) "
         "RETURN CASE n.birthyear WHEN 1952 THEN 1 WHEN 1960 THEN 2 ELSE 3 END AS type",
         1},
        {"MATCH (n) RETURN CASE n.name WHEN null THEN false ELSE true END AS hasName", 21},
        {"OPTIONAL MATCH (n {name:'Liam Neeson'}) RETURN CASE n WHEN null THEN false ELSE true END "
         "AS hasN",
         1},
        {"RETURN 2020", 1},
        {"RETURN 1+2+3-4", 1},
        {"RETURN 1+2 - (3+4)", 1},
        {"RETURN 1+2*3", 1},
        {"RETURN (2+15)/2-3*8%10 /*4.5*/", 1},
        {"WITH 2 AS a,15 AS b RETURN (a+b)/a-a*b%4", 1},
        {"RETURN 2^3", 1},
        {"RETURN 2^3^2", 1},
        {"RETURN 2^(1+2)^2", 1},
        {"RETURN 2^(1+2)*3^2-51/(8%5) /*55*/", 1},
        {"RETURN 1+2.0+3+4.0", 1},
        {"RETURN 1+'a'", 1},
        {"RETURN ['a']+'a'", 1},
        {"RETURN 1+[1]", 1},
        {"RETURN TRUE+[1.0]", 1},
        {"RETURN NULL+'a'", 1},
        {"RETURN 1+NULL", 1},
        {"RETURN 1.0-2+3.0", 1},
        {"RETURN NULL-1.1", 1},
        {"RETURN 1.0*2*3.0", 1},
        {"RETURN 1.0*NULL", 1},
        {"RETURN 1.0/2/3.0", 1},
        {"RETURN 1.0/NULL", 1},
        {"RETURN 5%3", 1},
        {"RETURN NULL%3", 1},
        {"RETURN 0^0", 1},
        {"RETURN null^3.0", 1},
        {"RETURN 0^null", 1},
        {"RETURN (2.0+1)*3-2/1.5+'a'+[1]", 1},
        {"RETURN null^2*3/4+5-6", 1},
        {"MATCH (n) WHERE n.name STARTS WITH 'Li' RETURN n,n.name", 2},
        {"MATCH (n) WHERE n.name ENDS WITH 'Redgrave' RETURN n,n.name", 5},
        {"MATCH (n) WHERE n.name CONTAINS 'Li' RETURN n,n.name", 2},
        {"MATCH (n) WHERE n.name CONTAINS 'Redgrave' RETURN n,n.name", 5},
        {"MATCH (n) WHERE n.name CONTAINS 'on' RETURN n,n.name", 5},
        {"MATCH (n) WHERE n.name CONTAINS 'cha' RETURN n,n.name", 3},
        {"MATCH (n) WHERE n.name STARTS WITH 'Li&alonglongstring' RETURN n,n.name", 0},
        {"MATCH (n) WHERE n.name ENDS WITH 'on&alonglongstring' RETURN n,n.name", 0},
        {"MATCH (n) WHERE n.name CONTAINS 'on&alonglongstring' RETURN n,n.name", 0},
        {"MATCH (n) WHERE n.name REGEXP '.*' RETURN n.name", 16},
        {"MATCH (n) WHERE n.name REGEXP 'Li.*' RETURN n.name", 2},
        {"MATCH (n) WHERE n.name REGEXP '.*Redgrave' RETURN n.name", 5},
        {"MATCH (n) WHERE n.name REGEXP 'Houston.*' RETURN n.name", 1},
        {"MATCH (n) WHERE n.name REGEXP 'L.*on' RETURN n.name", 2},
        {"MATCH (n) WHERE n.name REGEXP '.*ee.*' RETURN n.name", 1},
        /* named path */
        {"MATCH p = (n {name:'Rachel Kempson'})-[]->() RETURN p", 3},
        {"MATCH p = (n {name:'Rachel Kempson'})<-[]-() RETURN p", 1},
        {"MATCH p = (n {name:'Rachel Kempson'})-[]-() RETURN p", 4},
        {"MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]->() RETURN p", 8},
        {"MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]->(m) RETURN p,m", 8},
        {"MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]-() RETURN p", 11},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*1..2]->() RETURN p", 11},
        {"MATCH p = (n {name:'Rachel Kempson'})<-[*1..2]-() RETURN p", 3},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*1..2]-() RETURN p", 20},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*0..1]-() RETURN p", 5},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*2..3]-() RETURN p", 51},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*0..1]->()-[]->() RETURN p", 11},
        {"MATCH p = (n {name:'Rachel Kempson'})-[]->()-[]-() RETURN p,length(p)", 11},
        {"MATCH p = (n {name:'Rachel Kempson'})-[*0..3]->() RETURN p,length(p)", 21},
        /* null test */
        {"MATCH (n) WHERE n.name IS NULL RETURN n,label(n)", 5},
        {"MATCH (n) WHERE n.name IS NOT NULL RETURN n,label(n)", 16},
        {"MATCH (n) WHERE n.name IS NOT NULL AND n.birthyear IS NULL RETURN n,label(n)", 3},
        {"MATCH (n) RETURN n.name, "
         "CASE WHEN n.birthyear IS NULL THEN -1 "
         "ELSE n.birthyear + 10 END AS birth_10_years_later",
         21},
        /* exists test */
        {"MATCH (n) WHERE exists(n.title) RETURN n.title,n", 5},
        {"MATCH (n) RETURN exists(n.name) AS has_name,exists(n.title) AS has_title,label(n)", 21},
        {"MATCH (n) RETURN n,CASE exists(n.name) WHEN true THEN n.name ELSE n.title END AS content",
         21},
        /* relationships pattern expression */
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[]->()) /*true*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[]->(:Person)) /*true*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[]->(:City)) /*false*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED]->()) /*true*/", 1},
        // TODO(botu.wzy)
        // {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:NO_TYPE]->()) /*false*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}),(m {name:'Liam Neeson'}) RETURN exists((n)-[]->(m)) "
         "/*false*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}),(m {name:'Liam Neeson'}) RETURN NOT "
         "exists((n)-[]->(m)) "
         "AS isNew /*true*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}),(m {name:'Michael Redgrave'}) RETURN "
         "exists((n)-[]->(m)) "
         "/*true*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED]->()-[]->()) /*true*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED]->()-[:ACTED_IN]->()) "
         "/*true*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED]->()-[:BORN_IN]->()) "
         "/*false*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[*3]->()) /*true*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED*3]->()) /*false*/", 1},
        {"MATCH (n {name:'Rachel Kempson'}) RETURN exists((n)-[:MARRIED|HAS_CHILD*3]->()) /*true*/",
         1},
        {"MATCH (n {name:'Rachel Kempson'}),(m:Person) RETURN exists((n)-[]->(m)) /*3 true*/", 13},
        {"MATCH (n {name:'Rachel Kempson'}),(m:Person) RETURN exists((n)-[:MARRIED]->(m)) /*1 "
         "true*/",
         13},
        {"MATCH (n {name:'Rachel Kempson'}),(m:Person) RETURN exists((n)-[:MARRIED]->()-[]->(m)) "
         "/*3 "
         "true*/",
         13},
        {"MATCH (n {name:'Rachel Kempson'}),(m:Person) RETURN exists((n)-[:HAS_CHILD]->()-[]->(m)) "
         "/*2 true*/",
         13},
        {"MATCH (n {name:'Rachel Kempson'}),(m:City) RETURN "
         "exists((n)-[:HAS_CHILD]->()-[:BORN_IN]->(m)) /*1 true*/",
         3},
        {"MATCH (n:Person)-[:BORN_IN]->(c) WHERE exists((n)-[:HAS_CHILD]->()-[:BORN_IN]->(c)) "
         "RETURN "
         "n,c",
         1},
        // "MATCH (n {name:'Rachel Kempson'})-[r:MARRIED]->() RETURN exists((n)-[r]->())"
        /* list */
        {"RETURN [0, 1, 2, 3, 4, 5, 6, 7, 8, 9] AS list", 1},
        {"WITH [1,3,5,7] AS a RETURN a", 1},
        {"RETURN range(0, 4), range(2, 18, 3) /* [0,1,2,3,4],[2,5,8,11,14,17] */", 1},
        {"RETURN range(0, 10)[3]", 1},
        {"RETURN range(0, 10)[-3] /* 8 */", 1},
        {"RETURN range(0, 10)[15]", 1},
        {"RETURN range(0, 10)[0..3]", 1},
        {"RETURN range(0, 10)[0..-5] /* [0,1,2,3,4,5] */", 1},
        {"RETURN range(0, 10)[3..3] /* [] */", 1},
        {"RETURN range(0, 10)[3..1] /* [] */", 1},
        {"RETURN [1,2,3,4,5]+[6,7] AS myList", 1},
        {"WITH [1,2,3,4,5] AS l1, [6,7] AS l2 RETURN l1+l2 AS myList", 1},
        {"MATCH (n:City) RETURN collect(n.name)+['Wuhan'] AS cNames", 1},
        /* list test */
        {"UNWIND [2, 3, 4, 5] AS number WITH number WHERE number IN [2, 3, 8] RETURN number", 2},
        /* datetime, bool, binary */
        {"RETURN date() AS d", 1},
        {"RETURN date('2017-05-03')", 1},
        {"RETURN date('2017-05-01')", 1},
        {"RETURN date('2017-05-01') < date('2017-05-03')", 1},
        {"RETURN date('2017-05-03') > date('2017-05-01')", 1},
        {"RETURN date('2017-05-01') = date('2017-05-01')", 1},
        {"RETURN datetime() AS timePoint", 1},
        {"RETURN datetime('2017-05-03 10:40:32') AS timePoint", 1},
        {"RETURN datetime('2017-05-01 10:00:00') > datetime('2017-05-03 08:00:00')", 1},
        {"RETURN datetime('2017-05-01 10:00:00') < datetime('2017-05-03 08:00:00')", 1},
        {"RETURN datetime('2017-05-01 10:00:00.000001') > datetime('2017-05-01 10:00:00')", 1},
        {"RETURN datetime('2017-05-01 10:00:00.000002') > datetime('2017-05-01 10:00:00.000001')",
        1},
        {"WITH datetime('2017-05-01 10:00:00') AS t1, datetime('2017-05-03 08:00:00') AS t2 RETURN "
         "t1>t2,t1<t2",
         1},
        {"WITH true AS bt, false AS bf RETURN bt = false, bf = false, bt <> bf", 1},
        {"RETURN bin('MjAyMAo=') > bin('MjAxOQo=')", 1},
        {"RETURN bin('MjAyMAo=') < bin('MjAxOQo=')", 1},
        {"RETURN bin('MjAyMAo=') = bin('MjAyMAo=')", 1},
        {"WITH bin('MjAyMAo=') AS bin1, bin('MjAxOQo=') AS bin2 RETURN bin1>bin2,bin1<bin2", 1},
        /* datetime components */
        {"RETURN dateComponent(12345, 'year'), datetimeComponent(12345, 'month'), "
        "datetimeComponent(12345, 'day')",
        1},
        {"RETURN datetimeComponent(1582705717, 'year'),datetimeComponent(1582705717, "
         "'month'),datetimeComponent(1582705717, 'day')",
         1},
        {"RETURN datetimeComponent(1582705717, 'hour'),datetimeComponent(1582705717, "
         "'minute'),datetimeComponent(1582705717, 'second'), datetimeComponent(1582705717,"
         " 'microsecond')",
         1},
        {"RETURN datetimeComponent(1582705717000, 'year'),datetimeComponent(1582705717000, "
         "'second'), datetimeComponent(1582705717000, 'microsecond')",
         1},
        {"RETURN point('0101000020E6100000000000000000F03F0000000000000040') as p3", 1},
        {"RETURN linestring('0102000020231C000003000000000000000000000000000000000000000"
                            "00000000000004000000000000000400000000000000840000000000000F03F')", 1},
        {"RETURN polygon('0103000020E6100000010000000500000000000000000000000000000000"
                         "00000000000000000000000000000000001C400000000000001040000000000000"
                         "00400000000000000040000000000000000000000000000000000000000000000000')"
                         , 1},
        {"RETURN point(1.0, 2.0, 4326)", 1},   // (p1, p2), srid
        {"RETURN point(3.0, 1.0, 7203)", 1},
        {"RETURN point(2.32, 4.96)", 1},
        {"WITH point(1, 1, 4326) AS p1, point(1, 1, 4326) AS p2 RETURN p1 = p2", 1},
        {"RETURN pointwkb('0101000000000000000000F03F0000000000000040'), 4326", 1},
        {"RETURN pointwkt('POINT(1.0 1.0)'), 7203", 1},
        {"RETURN pointwkb('0101000000000000000000F03F0000000000000040')", 1},
        {"RETURN pointwkt('POINT(1.0 1.0)')", 1},
        {"RETURN linestringwkb('01020000000300000000000000000000000000000000"
        "000000000000000000004000000000000000400000000000000840000000000000F03F', 7203)",
        1},
        {"RETURN linestringwkt('LINESTRING(0 0,2 2,3 1)', 7203)", 1},
        {"RETURN polygonwkb('0103000000010000000500000000000000000000000000000000000000000"
        "00000000000000000000000001C4000000000000010400000000000000040"
        "0000000000000040000000000000000000000000000000000000000000000000', 4326)", 1},
        {"RETURN polygonwkt('POLYGON((0 0,0 7,4 2,2 0,0 0))', 7203)", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_with(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"match (n {name:'Liam Neeson'}) with n as n1\n"
         "match (n {name:'John Williams'}) return n,n1",
         1},
        {"match (n {name:'Liam Neeson'}) with n as n1\n"
         "match (n {name:'Dennis Quaid'}) with n as n2, n1\n"
         "match (n {name:'John Williams'}) return n,n2,n1",
         1},
        {"match (n {name:'Liam Neeson'}) with n as n1\n"
         "match (n {name:'Dennis Quaid'}) with n as n2, n1.name as n1name\n"
         "match (n {name:'John Williams'}) return n,n2,n1name",
         1},
        {"match (n {name:'Liam Neeson'}) with n return n", 1},
        // argument
        {"match (n {name:'Liam Neeson'}) with n\n"
         "match (n)-->(m) return n,m",
         2},
        {"match (n {name:'London'}) with n\n"
         "optional match (n)-->(m) return n,m",
         1},
        {"match (a {name:'Liam Neeson'})-[r]->(b) with b\n"
         "match (b)-[]->(c) return c",
         3},
        {"match (a {name:'Liam Neeson'}),(b {name:'London'}) with a, b\n"
         "match (c:Film) return a,b,c",
         5},
        {"match (n {name:'Liam Neeson'}) with n\n"
         "match (n) return n.name",
         1},
        {"match (a {name:'Liam Neeson'}), (b {name:'London'}) with a, b\n"
         "match (a), (b) return a.name, b.name",
         1},
        /* exception: unknown variable
       {"match (a {name:'Liam Neeson'})-[r]->(b) with b\n"
        "match (b)-[]->(c) return a,b,c", 3},
         */
        // aggregate as argument
        {"MATCH (a {name:'Liam Neeson'})-[r]->(b) RETURN a,count(b) AS out_num", 1},
        {"MATCH (a {name:'Liam Neeson'})-[r]->(b) WITH a,count(b) AS out_num\n"
         "MATCH (a)<-[]-(c) RETURN count(c) AS in_num,out_num",
         1},
        {"match (a {name:'Liam Neeson'})-[r]->(b) with a,b\n"
         "match (b)-[]->(c) return a,b,c",
         3},
        {"match (a {name:'Liam Neeson'})-[r]->(b) with a,a.name as root,b\n"
         "match (b)-[]->(c) return a,b,c",
         3},
        {"match (n {name:'Liam Neeson'}),(m {name:'Natasha Richardson'}),"
         "(n)-[r]->(m) return r,type(r)",
         1},
        {"match (n {name:'Liam Neeson'}),(m {name:'Natasha Richardson'}) with n,m\n"
         "match (n)-[r]->(m) return r,type(r)",
         1},  // todo: optimize, equivalent to the query above
        {"match (n {name:'Liam Neeson'}),(m {name:'Liam Neeson'}) with n,m\n"
         "optional match (n)-[r]->(m) return r,type(r)",
         1},
        {"match (n {name:'Liam Neeson'})-[r]->(m) with r return r,type(r)", 2},
#if 0
        {"match (n {name:'Liam Neeson'})-[r]->(m) with r\n"
             "match (a)-[r]->(b) return a,b,r", 2},
#endif
        {"match (n:City)\n"
         "with count (n) as num_city\n"
         "match (n:Film)\n"
         "return count(n) as num_film, num_city",
         1},
        {"match (n:Person {name:'Vanessa Redgrave'})-->(m)\n"
         "with m as m1\n"
         "match (n:Person {name:'Vanessa Redgrave'})<--(m)\n"
         "return m as m2, m1",
         6},
        {"match (n:Person {name:'Vanessa Redgrave'})-->(m)\n"
         "with count(m) as c1\n"
         "match (n:Person {name:'Vanessa Redgrave'})<--(m)\n"
         "return count(m) as c2, c1",
         1},
        {"match (n:Person {name:'Vanessa Redgrave'})-->(m)\n"
         "with count(m) as cm1\n"
         "match (n:Person {name:'Vanessa Redgrave'})<--(m)\n"
         "with count(m) as cm2, cm1\n"
         "match (n:Person {name:'Natasha Richardson'})-->(m)\n"
         "return count(m) as cm3, cm2, cm1",
         1},
        {"match (n:Person {name:'Michael Redgrave'})-->(m:Person)\n"
         "where m.birthyear > 1938 with count(m) as p38\n"
         "match (n:Person {name:'Michael Redgrave'})-->(m:Person)\n"
         "where m.birthyear > 1908 return count(m) as p08,p38 /* 3,1 */",
         1},
        /* WITH + WHERE */
        {"WITH 2020 AS x WHERE x > 2020 RETURN x", 0},
        {"MATCH (n:City) WITH 2020 AS x, n.name AS y ORDER BY y WHERE x = 2020 RETURN x,y", 3},
        {"MATCH (n) WITH n WHERE n.name = 'Liam Neeson' "
         "MATCH (m {name:'John Williams'}) RETURN n,m",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})-->(m:Person)\n"
         "WHERE m.birthyear > 1908 WITH count(m) AS p08\n"
         "RETURN p08 /* 3 */",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(m)\n"
         "WITH m, count(*) AS edge_num WHERE edge_num > 1.0\n"
         "RETURN m.name,edge_num",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(m)\n"
         "WITH n, m, count(*) AS edge_num WHERE edge_num > 1.0 OR n.birthyear > 1900\n"
         "RETURN m.name,edge_num",
         5},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(m)\n"
         "WITH m, count(*) AS edge_num WHERE edge_num > 1.0 AND m.birthyear > 1900\n"
         "RETURN m.name,edge_num",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(nbr)-->()\n"
         "WITH nbr, count(*) AS foaf WHERE foaf > 1.0\n"
         "RETURN nbr.name,foaf",
         2},
        {"MATCH (n:Person {name:'Michael Redgrave'})\n"
         "WHERE n.birthyear > 1900 AND n.birthyear < 2000\n"
         "RETURN n.name",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(m)\n"
         "WITH m, count(*) AS edge_num WHERE toInteger(edge_num) > 1\n"
         "RETURN m.name,edge_num",
         1},
        {"MATCH (n:Person {name:'Michael Redgrave'})--(nbr)-->()\n"
         "WITH nbr, count(*) AS foaf WHERE toInteger(foaf) > 1\n"
         "RETURN nbr.name,foaf",
         2},
        {"MATCH (a:City) WITH a MATCH (b:Person {name:'Liam Neeson'}) RETURN a,b", 3},
        /* Dynamic scan */
        {"WITH 'Vanessa Redgrave' AS varName MATCH (n:Film) RETURN n,varName", 5},
        {"WITH 'Vanessa Redgrave' AS varName MATCH (n {name:varName}) RETURN n", 1},
        {"MATCH (n {birthyear:1952}) WITH n,n.name AS varName MATCH (m {name:varName}) RETURN n,m",
         1},
        {"WITH 1 AS a MATCH (n:City) RETURN DISTINCT a,n", 3},
        /* WITH + OPTIONAL */
        {"MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m\n"
         "MATCH (m)-[:ACTED_IN]->(film) RETURN m.name,film",
         2},
        {"MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m\n"
         "OPTIONAL MATCH (m)-[:ACTED_IN]->(film) RETURN m.name,film",
         3},
        {"MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m\n"
         "OPTIONAL MATCH (m)-[:ACTED_IN]->(film)<-[:ACTED_IN]-(coactor) RETURN m.name,film,coactor",
         3},
        {"MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m\n"
         "OPTIONAL MATCH (m)-[:ACTED_IN]->(film) WITH m,film\n"
         "RETURN m.name,film",
         3},
        {"MATCH (n {name:'Rachel Kempson'})-[]->(m:Person) WITH m\n"
         "OPTIONAL MATCH (m)-[:ACTED_IN]->(film) WITH m,film\n"
         "OPTIONAL MATCH (film)<-[:WROTE_MUSIC_FOR]-(musician) RETURN m.name,film,musician",
         3},
        /* WITH + FILTER */
        {"match (n:Person) where n.name='Michael Redgrave' with n.birthyear as nb\n"
         "match (p)-[:HAS_CHILD]->(c) where p.birthyear=nb return c.name",
         2},
        {"match (n:Person) where n.name='Roy Redgrave' or n.name='Michael Redgrave' with "
         "collect(id(n)) as cn\n"
         "match (p:Person) where id(p) in cn return p.name",
         2},
        {"match (n:Person) where n.name='Roy Redgrave' or n.name='Michael Redgrave' with n, "
         "collect(id(n)) as cn\n"
         "match (p:Person) where id(p) in cn return p.name",
         2},
        {"match (c:Person)-[:HAS_CHILD]->(f:Person) where c.name='Roy Redgrave' with c, f\n"
         "match (m:Person)-[:ACTED_IN]->(film:Film)<-[:WROTE_MUSIC_FOR]-(p:Person) where "
         "m.name=f.name return c.name, p.name",
         1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_list_comprehension(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"RETURN [x IN range(0,10) | x] AS result", 1},
        {"RETURN [x IN range(0,10) | x^3] AS result", 1},
        {"WITH [2,4,6] AS y RETURN [x IN y | x] AS result /*[2,4,6]*/", 1},
        {"WITH [2,4,6] AS y RETURN [x IN range(0, size(y)) | x] AS result /*[0,1,2,3]*/", 1},
#if 0  // todo
        {"RETURN [x IN range(0,10) WHERE x % 2 = 0 | x] AS result", 1},
            {"RETURN [x IN range(0,10) WHERE x % 2 = 0 | x^3] AS result", 1},
#endif
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_profile(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "MATCH (n:Person) WHERE n.birthyear > 1960 RETURN n LIMIT 5",
        "PROFILE MATCH (n:Person) WHERE n.birthyear > 1960 RETURN n LIMIT 5",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_unwind(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        // premise: list expression test
        {"UNWIND [1, 2, 3] AS x RETURN x", 3},
        {"WITH [1, 1, 2, 2] AS coll UNWIND coll AS x RETURN x", 4},
        {"UNWIND $personIds AS personId RETURN personId", 3},
        {"UNWIND $personIds AS personId MATCH (n:Person {name:personId}) RETURN n", 3},
        {"UNWIND [] AS empty RETURN empty, 'literal_that_is_not_returned'", 0},
        {"UNWIND NULL AS x RETURN x, 'some_literal'", 0},
        {"UNWIND [1,2] AS x MATCH (n {name:'Houston'}) RETURN x,n", 2},
        {"UNWIND [1,2] AS x MATCH (n {name:'Houston'}),(m:Film) RETURN x,n,m", 10},
        {"UNWIND ['Paris','Houston'] AS x MATCH (n {name:x}),(m:Film) RETURN x,n,m", 5},
        {"MATCH (c {name:'Houston'}) WITH c MATCH (c)<-[r]-(p) RETURN p", 1},
        {"MATCH (c {name:'Houston'}) WITH c MATCH (p)-[r]->(c) RETURN p", 1},
        /* with arguments */
        {"MATCH (a {name:'Liam Neeson'}) WITH a,'London' AS cid MATCH (c {name:cid}) RETURN a,c",
         1},
        {"MATCH (a {name:'Liam Neeson'}) WITH a,['London','Houston'] AS cids\n"
         "UNWIND cids AS cid MATCH (c {name:cid}) RETURN a,count(c)",
         1},
        {"MATCH (a {name:'Liam Neeson'}),(b {name:'Dennis Quaid'}) WITH a,b,['London','Houston'] "
         "AS cids\n"
         "UNWIND cids AS cid MATCH (c {name:cid}) RETURN a,b,count(c)",
         1},
        {"MATCH (a {name:'Dennis Quaid'}) WITH a,['London','Houston'] AS cids\n"
         "UNWIND cids AS cid MATCH (c {name:cid})<-[]-(a) RETURN a,count(c)",
         1},
        {"MATCH (a {name:'Liam Neeson'}) WITH a,['London','Houston'] AS cids\n"
         "UNWIND cids AS cid MATCH (c {name:cid})<-[]-()-[:MARRIED]->(a) RETURN a,count(c)",
         1},
        /* write */
        {"MATCH (c {name:'Houston'}) WITH c "
         "MATCH (p:Person {name:'Liam Neeson'}) CREATE (c)-[:HAS_CHILD]->(p)",
         1},
        {"MATCH (c {name:'Houston'}) WITH c "
         "UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (c)-[:HAS_CHILD]->(q)",
         1},
        {"MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer1', "
         "birthyear:2002})-[r:BORN_IN]->(c) "
         "RETURN p,r,c",
         1},
        {"MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer2', "
         "birthyear:2002})-[r:BORN_IN]->(c) WITH p\n"
         "UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q)",
         1},
        {"MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer3', "
         "birthyear:2002})-[r:BORN_IN]->(c) WITH p\n"
         "UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q) WITH "
         "p\n"
         "UNWIND ['Liam Neeson'] AS sId MATCH (s:Person {name:sId}) CREATE (p)-[:DIRECTED]->(s) /* "
         "1,7 */",
         1},
        {"MATCH (c {name:'Houston'}) CREATE (p:Person {name:'passer4', "
         "birthyear:2002})-[r:BORN_IN]->(c) WITH p\n"
         "UNWIND $personIds AS pId MATCH (q:Person {name:pId}) CREATE (p)-[:HAS_CHILD]->(q) WITH "
         "p\n"
         "UNWIND [] AS sId MATCH (s:Person {name:sId}) CREATE (p)-[:DIRECTED]->(s) /* 1,4 */",
         1},
        {"WITH [1, 1, 2, 2] AS coll UNWIND coll AS x WITH x RETURN collect(x)", 1},
        {"WITH [1, 1, 2, 2] AS coll UNWIND coll AS x WITH x RETURN collect(DISTINCT x)", 1},
#if 0
        {"WITH [1, 1, 2, 2] AS coll UNWIND coll AS x\n"
             "WITH DISTINCT x RETURN collect(x) AS SET /* [1,2] */", 1},
#endif
        // unwind + with
        {"CREATE (:City {name:'Shanghai'}), (:City {name:'Zhongshan'}), (:Person "
         "{name:'Zhongshan'})",
         1},
        {"UNWIND ['Zhongshan'] AS x WITH x MATCH (a {name:x}) RETURN a,a.name", 2},
        {"UNWIND ['Zhongshan', 'Shanghai'] AS x WITH x MATCH (a {name:x}) RETURN a,a.name", 3},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_procedure(cypher::RTContext *ctx) {
    UT_LOG() << "Load Plugin File";
    std::ifstream f;
    std::string text;
    std::vector<std::pair<std::string, std::string>> plugin_info = {
        {"scan_graph", "../../test/test_procedures/scan_graph.cpp"},
        {"standard", "../../test/test_procedures/standard_result.cpp"},
    };
    std::vector<std::string> plugin_scripts;
    std::string encode;
    for (auto &i : plugin_info) {
        text.clear();
        f.open(i.second, std::ios::in);
        std::string buf;
        while (getline(f, buf)) {
            text += buf;
            text += "\n";
        }
        f.close();
        encode = lgraph_api::encode_base64(text);
        plugin_scripts.push_back(
            "CALL db.plugin.loadPlugin('CPP','" + i.first + "','" + encode + \
            "','CPP','" + i.first + "', true, 'v1')");
    }
    std::vector<std::pair<std::string, std::string>> multi_file_info = {
        {"multi_files_core.cpp", "../../test/test_procedures/multi_files_core.cpp"},
        {"multi_files.cpp", "../../test/test_procedures/multi_files.cpp"},
        {"multi_files.h", "../../test/test_procedures/multi_files.h"}
    };
    std::map<std::string, std::string> contents;
    for (auto &i : multi_file_info) {
        text.clear();
        f.open(i.second, std::ios::in);
        std::string buf;
        while (getline(f, buf)) {
            text += buf;
            text += "\n";
        }
        f.close();
        encode = lgraph_api::encode_base64(text);
        contents[i.first] = encode;
    }
    std::string cypher_q = FMA_FMT("CALL db.plugin.loadPlugin('CPP','multi', "
        "\\{`{}`: \"{}\", `{}`: \"{}\", `{}`: \"{}\"\\}, 'CPP','multi', true, 'v1')",
        "multi_files_core.cpp", contents["multi_files_core.cpp"],
        "multi_files.cpp", contents["multi_files.cpp"],
        "multi_files.h", contents["multi_files.h"]);
    plugin_scripts.push_back(cypher_q);
    eval_scripts(ctx, plugin_scripts);

    static std::vector<std::string> scripts = {
        "CALL db.createVertexLabel('Director', 'name', 'name', 'string', "
        "false, 'age', 'int16', true)",
        "CALL db.createVertexLabel('P2', 'flag1', 'flag1', 'bool', false, 'flag2', 'bool', true)",
        "CALL db.createEdgeLabel('LIKE', '[]')",

        "CALL db.addIndex('Person', 'birthyear', false)",
        "CAll db.subgraph([1,2,3])",
        "CALL db.vertexLabels",
        "CALL db.edgeLabels",
        "CALL db.indexes",
        "CALL db.warmup",
        // "CALL db.backup('/tmp')",  // disable for now
        "CALL dbms.procedures",
        "CALL dbms.procedures YIELD signature",
        "CALL dbms.procedures YIELD signature, name",
        /* graph management */
        "CALL dbms.graph.createGraph('demo1')",
        "CALL dbms.graph.listGraphs()",
        "CALL dbms.graph.deleteGraph('demo1')",
        "CALL dbms.graph.listGraphs()",
        /* user management */
        "CALL dbms.security.showCurrentUser()",
        "CALL dbms.security.changePassword('73@TuGraph','000')",
        "CALL dbms.security.changePassword('000','73@TuGraph')",
        "CALL dbms.security.createUser('guest1','123')",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.changeUserPassword('guest1','abc')",
        //            "CALL dbms.security.changeUserRole('guest1',true)",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.deleteUser('guest1')",
        "CALL dbms.security.listUsers()",
        /* access permission management */
        //            "CALL dbms.security.accessPermission('admin','default')",
        "CALL dbms.graph.createGraph('demo2')",
        "CALL dbms.security.createUser('guest2','123')",
        //            "CALL dbms.security.accessPermission('guest2','')",
        //            "CALL dbms.security.setAccessPermission('guest2','default','WRITE')",
        //            "CALL dbms.security.setAccessPermission('guest2','demo2','READ')",
        //            "CALL dbms.security.accessPermission('guest2','')",
        //            "CALL dbms.security.accessPermission('','default')",
        //            "CALL dbms.security.deleteAccessPermission('guest2','default')",
        //            "CALL dbms.security.accessPermission('guest2','')",
        // config
        "CALL dbms.system.info()",
        "CALL dbms.config.list()",
        // "CALL dbms.config.update({OPT_DB_ASYNC:true, OPT_TXN_OPTIMISTIC:true,
        // OPT_AUDIT_LOG_ENABLE:true, OPT_IP_CHECK_ENABLE:false})", // cannot run this here since it
        // will cause galaxy restart
        // ip whitelist
        "CALL dbms.security.listAllowedHosts()",
        "CALL dbms.security.addAllowedHosts('192.168.1.2', '192.168.1.3')",
        "CALL dbms.security.deleteAllowedHosts('192.168.1.2')",
        // add by jiazhenjiang
        "CALL dbms.security.createUser('guest1','123')",
        "CALL dbms.security.getUserInfo('guest1')",
        "CALL dbms.security.listRoles()",
        "CALL dbms.security.createRole('test_role', 'test desc')",
        "CALL dbms.security.getRoleInfo('test_role')",
        "CALL dbms.security.modRoleDesc('test_role', 'modify test desc')",
        "CALL dbms.security.disableRole('test_role', true)",
        "CALL dbms.security.getRoleInfo('test_role')",
        "CALL dbms.security.disableRole('test_role', false)",
        "CALL dbms.graph.createGraph('tgraph1', 'test graph tgraph1', 1)",
        "CALL dbms.graph.createGraph('tgraph2', 'test graph tgraph2', 2)",
        "CALL dbms.graph.createGraph('tgraph3', 'test graph tgraph3', 3)",
        "CALL dbms.security.rebuildRoleAccessLevel('test_role', {tgraph1: 'READ', tgraph2: "
        "'WRITE', tgraph3: 'FULL'})",
        "CALL dbms.security.getRoleInfo('test_role')",
        "CALL dbms.security.modRoleAccessLevel('test_role', {tgraph1: 'NONE', tgraph2: 'NONE', "
        "tgraph3: 'WRITE'})",
        "CALL dbms.security.getRoleInfo('test_role')",
        "CALL dbms.security.disableUser('guest1', true)",
        "CALL dbms.security.setUserDesc('guest1', 'modify guest1 desc')",
        "CALL dbms.security.setCurrentDesc('modify root desc')",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.createRole('test_role1', 'test desc')",
        "CALL dbms.security.createRole('test_role2', 'test desc')",
        "CALL dbms.security.createRole('test_role3', 'test desc')",
        "CALL dbms.security.listRoles()",
        "CALL dbms.security.rebuildUserRoles('guest1', ['test_role1', 'test_role2', 'test_role3'])",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.deleteUserRoles('guest1', ['test_role2', 'test_role3'])",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.addUserRoles('guest1', ['test_role2', 'test_role3'])",
        "CALL dbms.security.listUsers()",
        "CALL dbms.security.deleteRole('test_role')",
        "CALL dbms.security.deleteUser('guest1')",
        "CALL db.plugin.listPlugin('CPP', 'any')",
        "CALL db.listLabelIndexes('Person', 'vertex')",
        "CALL dbms.security.getUserPermissions('admin')",
        "CALL dbms.graph.getGraphInfo('default')",
#ifndef __SANITIZE_ADDRESS__
        "CALL db.plugin.listPlugin('PY', 'any')",
        "CALL db.plugin.loadPlugin('PY','countPerson','ZGVmIFByb2Nlc3MoZGIsIGlucHV0KToKIC"
        "AgIHR4biA9IGRiLkNyZWF0ZVJlYWRUeG4oKQogICAgaXQgPSB0eG4uR2V0VmVydGV4SXRlcmF0b3IoKQogICAgbiA"
        "9IDAKICAgIHdoaWxlIGl0LklzVmFsaWQoKToKICAgICAgICBpZiBpdC5HZXRMYWJlbCgpID09ICdQZXJzb24nOgog"
        "ICAgICAgICAgICBuID0gbiArIDEKICAgICAgICBpdC5OZXh0KCkKICAgIHJldHVybiAoVHJ1ZSwgc3RyKG4pKQ=='"
        ",'PY','count person',true, 'v1')",
        "CALL db.plugin.listPlugin('PY', 'any')",
        "CALL db.plugin.getPluginInfo('PY','countPerson')",
        "CALL db.plugin.getPluginInfo('PY','countPerson',true)",
#endif
        "CALL dbms.task.listTasks()",
        "CALL plugin.cpp.scan_graph({scan_edges:true,times:2})",
        "CALL plugin.cpp.standard({})",
    // "CALL dbms.task.terminateTask()",
#if 0
        /* call plugin */
            /* The plugin manager is disabled in embedded APIs, so please
             * test this with server.  */
            "CALL plugin.cpp.list",
            "CALL plugin.cpp.scan_graph",
            "CALL plugin.cpp.scan_graph({scan_edges:true,times:2})",
            /* call custom functions (implemented with plugins) */
            "MATCH (n:City) RETURN custom.SortStr(n.name)",
            "RETURN custom.BinVarFoo(5,9)",
#endif
        /* inquery call */
        "CALL dbms.procedures() YIELD name RETURN name,1",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2 {name:'Rachel Kempson'})\n"
        "CALL algo.shortestPath(n1,n2) YIELD nodeCount,totalCost RETURN nodeCount,totalCost /* "
        "2,1.0 */",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2 {name:'Rachel Kempson'})\n"
        "CALL algo.shortestPath(n1,n2) YIELD path RETURN path /* "
        "[V[vid0],E[vid0_vid1_type_eid],V[vid1]] */",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2 {name:'Houston'})\n"
        "CALL algo.shortestPath(n1,n2) YIELD nodeCount,totalCost RETURN nodeCount,totalCost /* "
        "6,5.0 */",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2:City)\n"
        "CALL algo.shortestPath(n1,n2) YIELD nodeCount,totalCost RETURN "
        "n2.name,nodeCount,totalCost /* 3 results */",
        // TODO(wt): remove the invalid results
        "MATCH (n1 {name:'Michael Redgrave'}),(n2:City)\n"
        "CALL algo.shortestPath(n1,n2,{maxHops:3}) YIELD nodeCount RETURN n2.name,nodeCount /* 2 "
        "results */",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2 {name:'Rachel Kempson'})\n"
        "CALL algo.shortestPath(n1,n2,{relationshipQuery:[{label:'HAS_CHILD'}]}) "
        "YIELD nodeCount,totalCost"
        " RETURN nodeCount,totalCost /* 3,2.0 */",
        "MATCH (n1 {name:'Michael Redgrave'}),(n2 {name:'Rachel Kempson'})\n"
        "CALL algo.shortestPath(n1,n2,{relationshipQuery:'HAS_CHILD'}) YIELD nodeCount,totalCost"
        " RETURN nodeCount,totalCost /* 3,2.0 */",
        "MATCH (n1 {name:'Corin Redgrave'}),(n2 {name:'London'})\n"
        "CALL algo.allShortestPaths(n1,n2) YIELD nodeIds,cost RETURN nodeIds,cost /* 2 */",
        "MATCH (n1 {name:'Corin Redgrave'}),(n2 {name:'Liam Neeson'})\n"
        "CALL algo.allShortestPaths(n1,n2) YIELD nodeIds,cost RETURN nodeIds,cost /* 4 */",
        "MATCH (n1 {name:'Corin Redgrave'}),(n2 {name:'Liam Neeson'})\n"
        "CALL algo.allShortestPaths(n1,n2) YIELD nodeIds,relationshipIds,cost RETURN "
        "nodeIds,relationshipIds,cost /* 4 */",
        "CALL algo.pagerank(10) YIELD node, pr RETURN node, pr ORDER by pr desc LIMIT 1 /* V[9], "
        "0.0187 */",
        "CALL algo.pagerank(10) YIELD node, pr RETURN node.name, pr LIMIT 1 /* Rachel Kempson"
        "0.010590751202103139 */",
        "CALL algo.pagerank(10) YIELD node, pr with node MATCH(node)-[r]->(n)"
        "return node, r, n LIMIT 1"
        "/* V[0] E[0_2_0_0] E[0_2_0_0] V[2] */",
        "CALL dbms.procedures() YIELD name, signature WHERE name='db.subgraph' RETURN signature",
        "CALL dbms.meta.count()",
        "CALL dbms.meta.countDetail()",
        "CALL dbms.meta.refreshCount()",
    };
    eval_scripts(ctx, scripts);

    std::vector<std::string> call_signatured_plugins_scripts;
    auto add_signatured_plugins = [&call_signatured_plugins_scripts](
                                      const std::string& name,
                                      const std::string& plugin_source_path) {
        std::ifstream f;
        f.open(plugin_source_path, std::ios::in);

        std::string buf;
        std::string text = "";
        while (getline(f, buf)) {
            text += buf;
            text += "\n";
        }
        f.close();
        std::string encoded = lgraph_api::encode_base64(text);
        call_signatured_plugins_scripts.emplace_back(
            FMA_FMT("CALL db.plugin.loadPlugin('CPP','{}','{}','CPP','{}', true, 'v2')",
                    name, encoded, name));
    };

    add_signatured_plugins("v2_test_path",
                           "../../test/test_procedures/v2_test_path.cpp");
    call_signatured_plugins_scripts.emplace_back(
        "MATCH (a:Person {name: \"Christopher Nolan\"}), (b:Person {name: \"Corin Redgrave\"}) "
        "CALL plugin.cpp.v2_test_path(a, b) YIELD length, nodeIds "
        "RETURN length, nodeIds AS path");

    add_signatured_plugins("v2_pagerank", "../../test/test_procedures/v2_pagerank.cpp");
    call_signatured_plugins_scripts.emplace_back(
        "CALL plugin.cpp.v2_pagerank(10) "
        "YIELD node, weight WITH node, weight "
        "MATCH(node)-[r]->(n) RETURN node, r, n, weight");

    call_signatured_plugins_scripts.emplace_back(
        "MATCH (a:Person {name: \"Christopher Nolan\"}), (b:Person {name: \"Corin Redgrave\"}) "
        "CALL plugin.cpp.v2_test_path(a, b) YIELD length, nodeIds "
        "WITH length, nodeIds "
        "UNWIND nodeIds AS id "
        "RETURN id, length");

    add_signatured_plugins("peek_some_node_salt",
        "../../test/test_procedures/peek_some_node_salt.cpp");
    call_signatured_plugins_scripts.emplace_back(
        "CALL plugin.cpp.peek_some_node_salt(10) "
        "YIELD node, salt WITH node, salt "
        "MATCH(node)-[r]->(n) RETURN node, r, n, salt");

    add_signatured_plugins("v2_path_process",
        "../../test/test_procedures/v2_path_process.cpp");
    call_signatured_plugins_scripts.emplace_back(
        "MATCH p = (n {name:\"Rachel Kempson\"})-[*0..3]->() "
        "CALL plugin.cpp.v2_path_process(nodes(p)) YIELD idSum "
        "RETURN idSum");

    add_signatured_plugins("v2_algo", "../../test/test_procedures/v2_algo.cpp");
    call_signatured_plugins_scripts.emplace_back(
        "CALL plugin.cpp.v2_algo() YIELD res RETURN res");
    call_signatured_plugins_scripts.emplace_back(
        "CALL plugin.cpp.v2_algo()");
    eval_scripts(ctx, call_signatured_plugins_scripts);
    return 0;
}

int test_add(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CREATE (passerA:Person {name:'Passerby A', birthyear:1983})\n"
        "CREATE (passerB:Person {name:'Passerby B', birthyear:1984})\n"
        "CREATE (passerA)-[:MARRIED]->(passerB),"
        "       (passerB)-[:MARRIED]->(passerA)",
        "MATCH (a:Person {name:'Lindsay Lohan'}), (b:Film {title:'The Parent Trap'})\n"
        "CREATE (a)-[r:DIRECTED]->(b)",
        "MATCH (a:Person {name:'Lindsay Lohan'})-[r]->(b:Film {title:'The Parent Trap'}) RETURN r",
        "MATCH (a:Film),(b:City) CREATE (a)-[:BORN_IN]->(b) /* 15 edges */",
        "CREATE (sy:City {name:'Sanya'}) RETURN sy,sy.name",
        "MATCH (a:Person {name:'Passerby A'}), (sy:City {name:'Sanya'})\n"
        "CREATE (a)-[r:BORN_IN]->(sy) RETURN a.name,r,sy.name",
        "MATCH (a:Person {name:'Passerby A'}), (sy:City {name:'Sanya'}) WITH a,sy\n"
        "CREATE (a)-[r:BORN_IN]->(sy)",
        "CREATE (passerC:Person {name:'Passerby C'})",
        "MATCH (p:Person {name:'Passerby C'}) RETURN exists(p.birthyear) /* false */",
        /* expression property */
        "WITH 'Passerby D' AS x, 2020 AS y CREATE (:Person {name:x, birthyear:y})",
        "MATCH (a {name:'Passerby A'}) CREATE (:Person {name:'Passerby E', birthyear:a.birthyear})",
        "MATCH (a {name:'Passerby A'}) CREATE (:Person {name:'Passerby F', "
        "birthyear:a.birthyear+24})",
        "MATCH (a {name:'Passerby A'}) CREATE (:Person {name:'Passerby G', birthyear:id(a)})",
#if 0
        "WITH 'Passerby D' AS x, 2020 AS y CREATE (:Person {name:x, birthyear:y+2})",
#endif
    };
    eval_scripts(ctx, scripts);
    return 0;
}

// NOTE: SET vertex/edge NULL is deprecated, use DELETE instead.
int test_set(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CALL db.createVertexLabel('Person', 'name', 'name', 'string', false, "
        "'age', 'int16', true, 'eyes', 'string', true, 'date', 'DATE', true)",
        "CALL db.createEdgeLabel('KNOWS', '[]', 'weight', 'int16', true)",
        R"(
CREATE (a:Person {name:'A', age:13, date:DATE('2023-07-23')})
CREATE (b:Person {name:'B', age:33, eyes:'blue'})
CREATE (c:Person {name:'C', age:44, eyes:'blue'})
CREATE (d:Person {name:'D', eyes:'brown'})
CREATE (e:Person {name:'E'})
CREATE (f:Person {name:'F', age:1})
CREATE (g:Person {name:'G', age:2})
CREATE (h:Person {name:'H', age:2})
CREATE (i1:Person {name:'I', age:3})
CREATE (a)-[:KNOWS {weight:10}]->(b),
       (a)-[:KNOWS {weight:15}]->(c),
       (a)-[:KNOWS {weight:40}]->(d),
       (b)-[:KNOWS {weight:20}]->(e),
       (c)-[:KNOWS {weight:12}]->(e),
       (f)-[:KNOWS {weight:0}]->(g),
       (f)-[:KNOWS {weight:0}]->(h),
       (f)-[:KNOWS {weight:0}]->(i1)
)",
        "MATCH (n:Person {name:'E'}) SET n.name='X'",
        "MATCH (n:Person {name:'A'}), (m:Person {name:'B'}) SET n.age=50 SET m.age=51",
        "MATCH (n:Person {name:'A'})-[e:KNOWS]->(m:Person) SET n.age=50 SET e.weight=50",
        "MATCH (n:Person {name:'A'})-[e:KNOWS]->(m:Person) SET n.age=50, e.weight=50",
        "MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m.age = 34",
        "MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m.age = id(n)",
        "MATCH (n:Person {name:'B'})<-[]-(m:Person) SET m = {age: 33}",
        "match (n) return n,properties(n) /*debug*/",
        // "MATCH (n:Person {name:'D'}),(m:Person {name:'X'}) SET n=m",
        "match (n) return n,properties(n) /*debug*/",
        "MATCH (n:Person {name:'X'}) SET n += {name:'Y', age:19}",
        "match (n) return n,properties(n) /*debug*/",
        "MATCH (n {name:'A'})-[r:KNOWS]->(m {name:'B'}) SET r.weight=11",
        "MATCH (n)-[r:KNOWS]->(m) WHERE r.weight=15 SET r += {weight:16}",
        "MATCH (n)-[r:KNOWS]->(m) WHERE r.weight=40 SET r.weight = r.weight + 1",
        "MATCH (n)-[r:KNOWS]->(m) WHERE r.weight=20 SET r=NULL",
        "match (n)-[r]->(m) return r,properties(r) /*debug*/",
        "MATCH (n:Person {name:'Y'}) SET n=NULL",
        "match (n) return n,properties(n) /*debug*/",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_delete(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CALL db.createVertexLabel('Person', 'name', 'name', 'string', "
        "false, 'age', 'int16', true, 'eyes', 'string', true)",
        "CALL db.createEdgeLabel('KNOWS', '[]', 'weight', 'int16', true)",
        R"(
CREATE (a:Person {name:'A', age:13})
CREATE (b:Person {name:'B', age:33, eyes:'blue'})
CREATE (c:Person {name:'C', age:44, eyes:'blue'})
CREATE (d:Person {name:'D', eyes:'brown'})
CREATE (e:Person {name:'E'})
CREATE (f:Person {name:'F', age:1})
CREATE (g:Person {name:'G', age:2})
CREATE (h:Person {name:'H', age:2})
CREATE (i1:Person {name:'I', age:3})
CREATE (a)-[:KNOWS {weight:10}]->(b),
       (a)-[:KNOWS {weight:15}]->(c),
       (a)-[:KNOWS {weight:40}]->(d),
       (b)-[:KNOWS {weight:20}]->(e),
       (b)-[:KNOWS {weight:25}]->(f),
       (c)-[:KNOWS {weight:12}]->(e),
       (d)-[:KNOWS {weight:4}]->(a),
       (f)-[:KNOWS {weight:0}]->(g),
       (f)-[:KNOWS {weight:0}]->(h),
       (f)-[:KNOWS {weight:0}]->(i1)
)",
        "match (n)-[r]->(m) return r,properties(r) /*debug*/",
        "MATCH (n {name:'D'}) DELETE n",
        "MATCH (n {name:'B'})-[r:KNOWS]->() DELETE r",
        "MATCH (n:Person {name:'F'})-[r:KNOWS]->(m:Person {name:'I'}) DELETE r",
        "MATCH (n:Person {name:'A'}),(m:Person {name:'C'}) WITH n,m MATCH (n)-[r]->(m) DELETE r",
        "match (n)-[r]->(m) return r,properties(r) /*debug*/",
        "MATCH (n:Person {name:'A'}) DELETE n",
        "MATCH (n:Person {name:'B'}) WITH n DELETE n",
        "match (n) return n,properties(n) /*debug*/",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_merge(cypher::RTContext *ctx) {
    static const std::vector<std::string> add = {
        "CALL db.createVertexLabel('Person', 'name', 'name', 'string', false, "
        "'birthyear', 'int16', "
        "true, 'gender', 'int8',   true)\n",  // birthyear and gender can be null
        "CALL db.createVertexLabel('City', 'name', 'name', 'string', "
        "false, 'area', 'double',false, 'population', 'double', true)\n",
        "CALL db.createEdgeLabel('Knows', '[]', 'intimacy', 'double', true)\n",
        "CALL db.createEdgeLabel('Livein', '[]')\n",
        "CALL db.addIndex('City', 'area', true)\n",
        "CREATE (n:Person {name: 'Liubei', birthyear: 161,gender:1})\n",
        "CREATE (n:Person {name: 'Caocao', birthyear: 155,gender:1})\n",
        "CREATE (n:Person {name: 'Sunquan', birthyear: 182,gender:1})\n",
        "CREATE (n:Person {name: 'Guanyu', birthyear: 160,gender:1})\n",
        "CREATE (n:Person {name: 'Zhangfei', birthyear: 153,gender:1})\n",
        "CREATE (n:City {name: 'Beijing', area:16410.54, population:2154.2})\n",
        "CREATE (n:City {name: 'Shanghai', area:6340.5, population:2423.78})\n",
    };
    eval_scripts(ctx, add);
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MERGE (n:Person {name:'Liubei'}) RETURN n.birthyear, n.gender\n", 1},  // Merge single
                                                                                 // node
                                                                                 // specifying
                                                                                 // both label
                                                                                 // and property
        {"MERGE (n:Person {name:'Zhugeliang'}) ON CREATE SET n.gender=1,n.birthyear=181 "
         "RETURN n.name\n",
         1},  // merge with on create
        {"MERGE (n:Person {name:'Liubei'}) ON MATCH SET n.birthyear=2010 RETURN "
         "n.birthyear\n",
         1},  // merge with macth // need to specific a property index
        {"MERGE(n:Person {name:'Liubei'}) ON CREATE SET n.gender=1 ON MATCH SET "
         "n.birthyear=2020 RETURN n.name, n.gender,n.birthyear\n",
         1},  // merge on create and
              // match a existed node
        {"MERGE(n:Person {name:'Huatuo'}) ON CREATE SET n.gender=1 ON MATCH SET "
         "n.birthyear=2020 RETURN n.name, n.gender,n.birthyear\n",
         1},  // merge on create and
              // match acreate  node
        {"MERGE(n:Person {name:'Liubei'}) ON MATCH SET n.gender=0,n.birthyear=2050 RETURN "
         "n.name, n.gender,n.birthyear\n",
         1},  // merge with on match setting multiple
              // properties // need to specific a property
              // index
        {"MATCH(n:Person {name:'Caocao'}), (m:Person {name:'Sunquan'}) MERGE "
         "(n)-[r:Knows{intimacy:0.6}]->(m) RETURN r.intimacy\n",
         1},  // Create a realtionship
        {"MATCH(n:Person {name:'Caocao'}), (m:Person {name:'Sunquan'}) MERGE "
         "(n)-[r:Knows]->(m) RETURN r.intimacy\n",
         1},  // Merge on a relationship  exit, return
              // bug, dispaly null
        {"MATCH (n:Person),(m:City) WHERE n.name='Caocao' AND m.name='Beijing' MERGE "
         "(n)-[r:Livein]->(m) RETURN r\n",
         1},
        {"MATCH (n:Person {name:'Caocao'}) MERGE (n)-[r:Knows]->(m:Person {name:'Sunquan'})"
         "RETURN r\n",
         1},
        {"MATCH (n:Person),(m:City) WHERE n.birthyear >= 160 AND m.name = 'Beijing' MERGE "
         "(n)-[r:Livein]->(m) RETURN r\n",
         4},
        {"MERGE (n:Person {name:'Caocao'})-[r:Knows]->(m:Person {name:'Caogai'})"
         "RETURN r\n",
         1},
        // {"MERGE (n:Person {gender:1}) RETURN n\n", ?},       // create index on gender first
        // this query should return
        // multiple nodes, but currently
        // only one is returned
        {"MERGE (n:Person {name:'Huatuo'}) RETURN n.name\n", 1},  // Merge using unique
                                                                  // constraints creates a new
                                                                  // node if no node is found
        {"MERGE (n:Person {name:'Xunyu'}) RETURN n.name\n", 1},   // Merge using unique
                                                                  // constraints creates a new
                                                                  // node if no node is found
        {"MERGE (n:Person {name:'Liubei'}) RETURN n.birthyear, n.gender\n", 1},  // Merge using
                                                                                 // unique
                                                                                 // constraints
                                                                                 // matches an
                                                                                 // existing node
        // "MERGE (n:City {name: 'Beijing', area:1000,population:2154.2}) RETURN n\n",
        // Merge with unique constraints and partial matches , error information "MERGE
        // (n:City {name: 'Beijing', area:16410.54,population:2154.2}) RETURN
        // n.name,n.area\n", "MERGE (n:City {name: 'Beijing', area:6340.5,population:2154.2})
        // RETURN n\n",//Merge with unique constraints and partial matches , error
        // information
        {"MERGE (node1: Person {name: 'lisi'}) ON CREATE SET node1.birthyear = 1903 WITH "
         "node1 MATCH (node1) WHERE node1.birthyear < 1904 SET node1.birthyear = 1904 "
         "RETURN id(node1), node1.name, node1.birthyear\n",
         1},
        {"MERGE (n: Person {name: 'wangwu'}) ON CREATE SET n.birthyear = 1903 ON CREATE SET "
         "n.name = 'wangwu2' WITH n MATCH (n) WHERE n.birthyear < 2002 SET n += {birthyear: "
         "2002, name: 'wangwu2'} RETURN id(n), n.name, n.birthyear\n",
         1},
        {"MERGE (a:Person {name: 'zhangsan'}) SET a.birthyear = 2020 RETURN a.birthyear", 1},
        {"MERGE (a:Person {name: 'zhangsan'}) DELETE a", 1},
        {"MERGE (a:Person {name: 'zhangsan'}) CREATE (b:Person {name : 'xiaoming'})"
         "RETURN b",
         1},
        {"MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) RETURN n,m", 1},
        {"MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) RETURN n,m", 1},
        {"MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) CREATE "
         "(n)-[r:Knows]->(m) RETURN n, r, m",
         1},
        {"MERGE (n:Person {name:'zhangsan'}) MERGE (m:Person {name:'lisi'}) MERGE "
         "(n)-[r:Knows]->(m) return n, r, m",
         1},
        {"MATCH (a:Person {name:'zhangsan'}) SET a.birthyear = 2023 CREATE (b:Person "
         "{name:'wangwu'}) RETURN b",
         1},
        {"MATCH (a:Person {name:'zhangsan'}) SET a.birthyear = 2023 MERGE (b:Person "
         "{name:'wangwu'}) RETURN b",
         1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

bool Trts() { return false; }

int test_remove(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "MATCH (a {name:'Liam Neeson'}) REMOVE a.birthyear RETURN a.name,a.birthyear",
        "MATCH (a {name:'Liam Neeson'}) REMOVE a.name RETURN a.name,a.birthyear /* exception */",
    };
    UT_EXPECT_ANY_THROW(eval_scripts(ctx, scripts));
    return 0;
}

int test_order_by(cypher::RTContext *ctx) {
    static const std::vector<std::string> add = {
        "MATCH (a:Person {name:'Lindsay Lohan'}), (b:Film {title:'The Parent Trap'})\n"
        "CREATE (a)-[r:DIRECTED]->(b)",
    };
    eval_scripts(ctx, add);
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(v2) as cnt", 5},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as "
         "cnt",
         5},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as "
         "cnt order by cnt",
         5},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as "
         "cnt order by cnt desc",
         5},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as "
         "cnt order by cnt desc,v1.title",
         5},
        {"match (v1:Film) return distinct v1.title order by v1.title", 5},
        {"match (v1:Film) return distinct v1.title order by v1.title limit 3 /* Batman, Camelot, "
         "Goodbye */",
         3},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as "
         "cnt order by cnt desc limit 3"
         " /* NOTE: unstable heap sort */",
         3},
        {"match (:Person {name:'Vanessa Redgrave'})<-[:HAS_CHILD]-(p)-[:ACTED_IN*0..]->(m) "
         "return p.name,m order by p.name",
         3},
        {"MATCH (n) RETURN n,n.name AS name ORDER BY name", 21},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.name", 12},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name", 8},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER "
         "BY m.name",
         8},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER "
         "BY m.name LIMIT 5",
         5},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER "
         "BY m.name SKIP 2",
         6},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN DISTINCT m.name ORDER "
         "BY m.name SKIP 2 LIMIT 3",
         3},
        {"MATCH (v1:Film)<-[r:ACTED_IN|DIRECTED]-(v2:Person) RETURN v1.title AS title, r ORDER BY "
         "title LIMIT 5",
         5},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_topn(cypher::RTContext *ctx) {
    static const std::vector<std::string> add = {
        "MATCH (a:Person {name:'Lindsay Lohan'}), (b:Film {title:'The Parent Trap'})\n"
        "CREATE (a)-[r:DIRECTED]->(b)",
    };
    eval_scripts(ctx, add);
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"match (v1:Film) return v1.title order by v1.title limit 3", 3},
        {"match (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,v2.name as cnt order "
         "by cnt desc limit 3",
         3},
        {"match (:Person {name:'Vanessa Redgrave'})<-[:HAS_CHILD]-(p)-[:ACTED_IN*0..]->(m) "
         "return p.name,m order by p.name limit 3",
         3},
        {"MATCH (n) RETURN n.name AS name ORDER BY name LIMIT 10", 10},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.birthyear, m.name "
         "ORDER BY m.name LIMIT 5",
         5},
        {"MATCH (n:Person {name:'Vanessa Redgrave'})-[*2]-(m:Person) RETURN m.birthyear, m.name "
         "ORDER BY m.birthyear desc LIMIT 5",
         5},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_create_label(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CALL db.createLabel('vertex','Person', 'name', ['name', 'string', false], ['birthyear', "
        "'int16', true])",
        "CALL db.createLabel('vertex', 'City','name', ['name', 'string', false])",
        "CALL db.createLabel('vertex', 'Film', 'title', ['title', 'string', false])",
        "CALL db.createLabel('edge', 'HAS_CHILD', '[]')",
        "CALL db.createLabel('edge', 'MARRIED', '[]')",
        "CALL db.createLabel('edge', 'BORN_IN', '[]')",
        "CALL db.createLabel('edge', 'DIRECTED', '[]')",
        "CALL db.createLabel('edge', 'WROTE_MUSIC_FOR', '[]')",
        "CALL db.createLabel('edge', 'ACTED_IN', '[]', ['charactername', 'string', true])",
        R"(
CREATE (rachel:Person {name: 'Rachel Kempson', birthyear: 1910})
CREATE (michael:Person {name: 'Michael Redgrave', birthyear: 1908})
CREATE (vanessa:Person {name: 'Vanessa Redgrave', birthyear: 1937})
CREATE (corin:Person {name: 'Corin Redgrave', birthyear: 1939})
CREATE (liam:Person {name: 'Liam Neeson', birthyear: 1952})
CREATE (natasha:Person {name: 'Natasha Richardson', birthyear: 1963})
CREATE (richard:Person {name: 'Richard Harris', birthyear: 1930})
CREATE (dennis:Person {name: 'Dennis Quaid', birthyear: 1954})
CREATE (lindsay:Person {name: 'Lindsay Lohan', birthyear: 1986})
CREATE (jemma:Person {name: 'Jemma Redgrave', birthyear: 1965})
CREATE (roy:Person {name: 'Roy Redgrave', birthyear: 1873})

CREATE (john:Person {name: 'John Williams', birthyear: 1932})
CREATE (christopher:Person {name: 'Christopher Nolan', birthyear: 1970})

CREATE (newyork:City {name: 'New York'})
CREATE (london:City {name: 'London'})
CREATE (houston:City {name: 'Houston'})

CREATE (mrchips:Film {title: 'Goodbye, Mr. Chips'})
CREATE (batmanbegins:Film {title: 'Batman Begins'})
CREATE (harrypotter:Film {title: 'Harry Potter and the Sorcerer\'s Stone'})
CREATE (parent:Film {title: 'The Parent Trap'})
CREATE (camelot:Film {title: 'Camelot'})

CREATE (rachel)-[:HAS_CHILD]->(vanessa),
       (rachel)-[:HAS_CHILD]->(corin),
       (michael)-[:HAS_CHILD]->(vanessa),
       (michael)-[:HAS_CHILD]->(corin),
       (corin)-[:HAS_CHILD]->(jemma),
       (vanessa)-[:HAS_CHILD]->(natasha),
       (roy)-[:HAS_CHILD]->(michael),

       (rachel)-[:MARRIED]->(michael),
       (michael)-[:MARRIED]->(rachel),
       (natasha)-[:MARRIED]->(liam),
       (liam)-[:MARRIED]->(natasha),

       (vanessa)-[:BORN_IN]->(london),
       (natasha)-[:BORN_IN]->(london),
       (christopher)-[:BORN_IN]->(london),
       (dennis)-[:BORN_IN]->(houston),
       (lindsay)-[:BORN_IN]->(newyork),
       (john)-[:BORN_IN]->(newyork),

       (christopher)-[:DIRECTED]->(batmanbegins),

       (john)-[:WROTE_MUSIC_FOR]->(harrypotter),
       (john)-[:WROTE_MUSIC_FOR]->(mrchips),

       (michael)-[:ACTED_IN {charactername: 'The Headmaster'}]->(mrchips),
       (vanessa)-[:ACTED_IN {charactername: 'Guenevere'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'King Arthur'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'Albus Dumbledore'}]->(harrypotter),
       (natasha)-[:ACTED_IN {charactername: 'Liz James'}]->(parent),
       (dennis)-[:ACTED_IN {charactername: 'Nick Parker'}]->(parent),
       (lindsay)-[:ACTED_IN {charactername: 'Halle/Annie'}]->(parent),
       (liam)-[:ACTED_IN {charactername: 'Henri Ducard'}]->(batmanbegins)
)",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_create_yago(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CALL db.createVertexLabel('Person', 'name', 'name', "
        "'string', false, 'birthyear', 'int16', true)",
        "CALL db.createVertexLabel('City', 'name', 'name', 'string', false)",
        "CALL db.createVertexLabel('Film', 'title', 'title', 'string', false)",
        "CALL db.createEdgeLabel('HAS_CHILD', '[]')",
        "CALL db.createEdgeLabel('MARRIED', '[]')",
        "CALL db.createEdgeLabel('BORN_IN', '[]')",
        "CALL db.createEdgeLabel('DIRECTED', '[]')",
        "CALL db.createEdgeLabel('WROTE_MUSIC_FOR', '[]')",
        "CALL db.createEdgeLabel('ACTED_IN', '[]', 'charactername', 'string', false)",
        R"(
CREATE (rachel:Person {name: 'Rachel Kempson', birthyear: 1910})
CREATE (michael:Person {name: 'Michael Redgrave', birthyear: 1908})
CREATE (vanessa:Person {name: 'Vanessa Redgrave', birthyear: 1937})
CREATE (corin:Person {name: 'Corin Redgrave', birthyear: 1939})
CREATE (liam:Person {name: 'Liam Neeson', birthyear: 1952})
CREATE (natasha:Person {name: 'Natasha Richardson', birthyear: 1963})
CREATE (richard:Person {name: 'Richard Harris', birthyear: 1930})
CREATE (dennis:Person {name: 'Dennis Quaid', birthyear: 1954})
CREATE (lindsay:Person {name: 'Lindsay Lohan', birthyear: 1986})
CREATE (jemma:Person {name: 'Jemma Redgrave', birthyear: 1965})
CREATE (roy:Person {name: 'Roy Redgrave', birthyear: 1873})

CREATE (john:Person {name: 'John Williams', birthyear: 1932})
CREATE (christopher:Person {name: 'Christopher Nolan', birthyear: 1970})

CREATE (newyork:City {name: 'New York'})
CREATE (london:City {name: 'London'})
CREATE (houston:City {name: 'Houston'})

CREATE (mrchips:Film {title: 'Goodbye, Mr. Chips'})
CREATE (batmanbegins:Film {title: 'Batman Begins'})
CREATE (harrypotter:Film {title: 'Harry Potter and the Sorcerer\'s Stone'})
CREATE (parent:Film {title: 'The Parent Trap'})
CREATE (camelot:Film {title: 'Camelot'})

CREATE (rachel)-[:HAS_CHILD]->(vanessa),
       (rachel)-[:HAS_CHILD]->(corin),
       (michael)-[:HAS_CHILD]->(vanessa),
       (michael)-[:HAS_CHILD]->(corin),
       (corin)-[:HAS_CHILD]->(jemma),
       (vanessa)-[:HAS_CHILD]->(natasha),
       (roy)-[:HAS_CHILD]->(michael),

       (rachel)-[:MARRIED]->(michael),
       (michael)-[:MARRIED]->(rachel),
       (natasha)-[:MARRIED]->(liam),
       (liam)-[:MARRIED]->(natasha),

       (vanessa)-[:BORN_IN]->(london),
       (natasha)-[:BORN_IN]->(london),
       (christopher)-[:BORN_IN]->(london),
       (dennis)-[:BORN_IN]->(houston),
       (lindsay)-[:BORN_IN]->(newyork),
       (john)-[:BORN_IN]->(newyork),

       (christopher)-[:DIRECTED]->(batmanbegins),

       (john)-[:WROTE_MUSIC_FOR]->(harrypotter),
       (john)-[:WROTE_MUSIC_FOR]->(mrchips),

       (michael)-[:ACTED_IN {charactername: 'The Headmaster'}]->(mrchips),
       (vanessa)-[:ACTED_IN {charactername: 'Guenevere'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'King Arthur'}]->(camelot),
       (richard)-[:ACTED_IN {charactername: 'Albus Dumbledore'}]->(harrypotter),
       (natasha)-[:ACTED_IN {charactername: 'Liz James'}]->(parent),
       (dennis)-[:ACTED_IN {charactername: 'Nick Parker'}]->(parent),
       (lindsay)-[:ACTED_IN {charactername: 'Halle/Annie'}]->(parent),
       (liam)-[:ACTED_IN {charactername: 'Henri Ducard'}]->(batmanbegins)
)",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_aggregate(cypher::RTContext *ctx) {
    static const std::vector<std::string> create = {
        "CALL db.createVertexLabel('Person', 'name', 'name', "
        "'string', false, 'age', 'int16', true, "
        "'eyes', 'string', true)",
        "CALL db.createEdgeLabel('KNOWS', '[]')",
        R"(
CREATE (a:Person {name:'A', age:13})
CREATE (b:Person {name:'B', age:33, eyes:'blue'})
CREATE (c:Person {name:'C', age:44, eyes:'blue'})
CREATE (d:Person {name:'D', eyes:'brown'})
CREATE (e:Person {name:'E'})
CREATE (a)-[:KNOWS]->(b),
       (a)-[:KNOWS]->(c),
       (a)-[:KNOWS]->(d),
       (b)-[:KNOWS]->(e),
       (c)-[:KNOWS]->(e)
)",
    };
    eval_scripts(ctx, create);
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH (n:Person) RETURN avg(n.age) /* 30.0 */", 1},
        {"MATCH (n { name: 'A' })-->(x) RETURN count(x) /* 3 */", 1},
        {"MATCH (n:Person) RETURN count(n.age) /* 3 */", 1},
        {"MATCH (n:Person) RETURN max(n.age) /* 44 */", 1},
        {"MATCH (n:Person) RETURN min(n.age) /* 13 */", 1},
        {"MATCH (n:Person) RETURN percentileCont(n.age, 0.4) /* 29 */", 1},
        {"MATCH (n:Person) RETURN percentileDisc(n.age, 0.5) /* 33 */", 1},
        {"MATCH (n:Person) RETURN stDev(n.age) /* 15.716234 */", 1},
        {"MATCH (n:Person) RETURN stDevP(n.age) /* 12.832251 */", 1},
        {"MATCH (n:Person) RETURN variance(n.age)", 1},
        {"MATCH (n:Person) RETURN varianceP(n.age)", 1},
        {"MATCH (n:Person) RETURN collect(n.age) /* 13,33,44 */", 1},
        {"MATCH (n:Person) RETURN collect([n.name,n.age]) /* [[A, 13], [B, 33], [C, 44], [D, "
         "null], [E, null]] */",
         1},
        {"MATCH (n {name: 'A'})-[]->(x) RETURN label(n), n.age, count(*) /* Person,13,3.000000 */",
         1},
        {"MATCH (n {name: 'A'})-[]->(x) RETURN label(n), n, count(*) /* Person,V[0],3.000000 */",
         1},
        {"MATCH (n {name: 'A'})-[r]->() RETURN type(r), count(*) /* KNOWS,3.00000 */", 1},
        /* filter with OR */
        {"MATCH (n:Person) WHERE n.age = 13 OR n.age > 40 RETURN count(n) AS nCount", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_algo(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "CALL db.createVertexLabel('Loc', 'name', 'name', 'string', false)",
        "CALL db.createEdgeLabel('ROAD', '[]', 'cost', 'float', false)",
        R"(
CREATE (a:Loc {name:'A'})
CREATE (b:Loc {name:'B'})
CREATE (c:Loc {name:'C'})
CREATE (d:Loc {name:'D'})
CREATE (e:Loc {name:'E'})
CREATE (f:Loc {name:'F'})
CREATE (g:Loc {name:'G'})

CREATE (a)-[:ROAD {cost:50}]->(b)
CREATE (a)-[:ROAD {cost:50}]->(c)
CREATE (a)-[:ROAD {cost:100}]->(d)
CREATE (b)-[:ROAD {cost:40}]->(d)
CREATE (c)-[:ROAD {cost:40}]->(d)
CREATE (c)-[:ROAD {cost:80}]->(e)
CREATE (d)-[:ROAD {cost:30}]->(e)
CREATE (d)-[:ROAD {cost:80}]->(f)
CREATE (e)-[:ROAD {cost:40}]->(f);
)",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'F'})\n"
        "CALL algo.shortestPath(n1, n2) YIELD nodeCount RETURN nodeCount",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'G'})\n"
        "CALL algo.shortestPath(n1, n2) YIELD nodeCount RETURN nodeCount",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'E'})\n"
        "CALL algo.allShortestPaths(n1, n2) YIELD nodeIds,relationshipIds,cost RETURN "
        "nodeIds,relationshipIds,cost",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'E'})\n"
        "CALL algo.allShortestPaths(n1, n2) YIELD nodeIds,relationshipIds,cost WITH "
        "nodeIds,relationshipIds,cost\n"
        "UNWIND relationshipIds AS rid\n"
        "CALL algo.native.extract(rid, {isNode:false, field:'cost'}) YIELD value RETURN value",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'E'})\n"
        "CALL algo.allShortestPaths(n1, n2, {relationshipQuery:[{label:'ROAD'}]}) YIELD "
        "nodeIds,relationshipIds WITH nodeIds,relationshipIds\n"
        "UNWIND relationshipIds AS rid\n"
        "CALL algo.native.extract(rid, {isNode:false, field:'cost'}) YIELD value RETURN nodeIds, "
        "sum(value) AS score",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'E'})\n"
        "CALL algo.allShortestPaths(n1, n2, {relationshipQuery:'ROAD'}) YIELD "
        "nodeIds,relationshipIds WITH nodeIds,relationshipIds\n"
        "UNWIND relationshipIds AS rid\n"
        "CALL algo.native.extract(rid, {isNode:false, field:'cost'}) YIELD value RETURN nodeIds, "
        "sum(value) AS score",
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'E'})\n"
        "CALL algo.allShortestPaths(n1, n2, {relationshipQuery:[{label:'ROAD'}]}) YIELD "
        "nodeIds,relationshipIds,cost WITH nodeIds,relationshipIds,cost\n"
        "UNWIND relationshipIds AS rid\n"
        "CALL algo.native.extract(rid, {isNode:false, field:'cost'}) YIELD value WITH nodeIds, "
        "sum(value) AS score\n"
        "CALL algo.native.extract(nodeIds, {isNode:true, field:'name'}) YIELD value RETURN value, "
        "score",
#if 0
        "MATCH (n1:Loc {name:'A'}), (n2:Loc {name:'F'})\n"
            "CALL algo.shortestPath(n1, n2, 'cost') YIELD totalCost RETURN totalCost",
#endif
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_circle(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "MATCH (a)-[]->(b)-[]->(c)<-[]-(a) RETURN a,b,c",
        "MATCH (a:Person {birthyear:1908})-[]->(b)-[]->(c)<-[]-(a:Person {birthyear:1910}) RETURN "
        "a,b,c",  // not recommended
    };
    CYPHER_TODO();
    /**
     * The execution plan of the (a)-[]->(b)-[]->(c)<-[]-(a) is:
     *      produce results
     *          | expand into (a->c)
     *              | expand all (b->c)
     *              |   | expand all (a->b)
     *              |-------| scan (a)
     * The scan a operation is child of both expand all and expand into, so stream <scan> and
     * <expand all>-<expand all>-<scan> will effect each other.
     * While we assume op streams are independent in the execution machine.
     */
    eval_scripts(ctx, scripts);
    return 0;
}

void test_error_report(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        "MATCH (m)-[]->(n) WHERE m<>n RETURN m,n /* Expected Exception */",
    };
    UT_EXPECT_ANY_THROW(eval_scripts(ctx, scripts));
}

// TODO(someone): independent test case
int test_ldbc_snb(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts = {
        /* todo: native CREATE Label */
        "CALL db.createVertexLabel('TagClass', 'name', 'name', 'string', false)",
        "CALL db.createVertexLabel('Tag', 'name', 'name', 'string', false)",
        "CALL db.createVertexLabel('Forum', 'name', 'name', 'string', false)",
        "CALL db.createVertexLabel('Person', 'id', 'id', 'int32', false, "
        "'birthday', 'string', true)",
        "CALL db.createVertexLabel('Comment', 'id', 'id', 'int32', false)",
        "CALL db.createVertexLabel('Message', 'id', 'id', 'int32', false)",
        "CALL db.createEdgeLabel('hasTag', '[]')",
        "CALL db.createEdgeLabel('hasType', '[]')",
        "CALL db.createEdgeLabel('knows', '[]')",
        "CALL db.createEdgeLabel('hasMember', '[]')",
        "CALL db.createEdgeLabel('hasCreator', '[]')",
        "CALL db.createEdgeLabel('replyOf', '[]')",
        R"(
CREATE
  // tag classes, tags and forums
  (tagClass1:TagClass {name: 'MusicalArtist'}),
  (tagClass2:TagClass {name: 'OfficeHolder'}),
  (tag1:Tag {name: 'Elvis Presley'}),
  (tag2:Tag {name: 'Mr. Office'}),
  (forum1:Forum {name: 'Presley fan club'}),
  (forum2:Forum {name: 'Long live the closed office'}),
  // hasTag and hasType relationships
  (forum1)-[:hasTag]->(tag1)-[:hasType]->(tagClass1),
  (forum2)-[:hasTag]->(tag2)-[:hasType]->(tagClass2),
  // persons
  (person1:Person {id: 1, birthday: '1990-01-01'}),
  (person2:Person {id: 2, birthday: '1989-01-01'}),
  (person3:Person {id: 3, birthday: '1989-01-01'}),
  (person4:Person {id: 4, birthday: '1989-01-01'}),
  // knows relationships: person1 and persons 3/4 are strangers
  (person1)-[:knows]->(person2),
  (person2)-[:knows]->(person3),
  (person3)-[:knows]->(person4),
  // memberships
  (forum1)-[:hasMember]->(person3)<-[:hasMember]-(forum2),
  (forum1)-[:hasMember]->(person4)<-[:hasMember]-(forum2),
  // interactions
  (comment1:Comment {id: 1}),
  (comment2:Comment {id: 2}),
  (comment3:Comment {id: 3}),
  (message1:Message {id: 1}),
  (message2:Message {id: 2}),
  (message3:Message {id: 3}),
  (person1)<-[:hasCreator]-(comment1)-[:replyOf]->(message1)-[:hasCreator]->(person3),
  (person3)<-[:hasCreator]-(comment2)-[:replyOf]->(message2)-[:hasCreator]->(person1),
  (person4)<-[:hasCreator]-(comment3)-[:replyOf]->(message3)-[:hasCreator]->(person1)
)",
    };
    eval_scripts(ctx, scripts);
    return 0;
}

int test_opt(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH ()-[r]->() RETURN count(r) /* 28 */", 1},
        {"MATCH (:Person)-[r]->() RETURN count(r) /* 28 */", 1},
        {"MATCH (:Film)-[r]->() RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r]->(:Person) RETURN count(r) /* 11 */", 1},
        {"MATCH ()-[r]->(:Film) RETURN count(r) /* 11 */", 1},
        {"MATCH (:Person)-[r]->(:Film) RETURN count(r) /* 11 */", 1},
        // TODO(botu.wzy)
        // {"MATCH (:Person)-[r]->(:NO_LABEL) RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r:MARRIED]->() RETURN count(r) /* 4 */", 1},
        // TODO(botu.wzy)
        // {"MATCH ()-[r:NO_LABEL]->() RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r:MARRIED|BORN_IN]->() RETURN count(r) /* 10 */", 1},
        // TODO(botu.wzy)
        // {"MATCH ()-[r:MARRIED|BORN_IN|NO_LABEL]->() RETURN count(r) /* 10 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]->() RETURN count(r) /* 10 */", 1},
        {"MATCH (:City)-[r:MARRIED|BORN_IN]->() RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r:MARRIED|BORN_IN]->(:Person) RETURN count(r) /* 4 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]->(:City) RETURN count(r) /* 6 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]->(:Film) RETURN count(r) /* 0 */", 1},
        {"MATCH (:Person)-[r:DIRECTED]->(:Film) RETURN count(r) /* 1 */", 1},
        {"MATCH (:Person)-[r:DIRECTED|ACTED_IN]->(:Film) RETURN count(r) /* 9 */", 1},
        /* bi direction */
        {"MATCH ()-[r]-() RETURN count(r) /* 28 * 2 */", 1},
        {"MATCH (:Person)-[r]-() RETURN count(r) /* 39 */", 1},
        {"MATCH (:Film)-[r]-() RETURN count(r) /* 11 */", 1},
        {"MATCH ()-[r]-(:Person) RETURN count(r) /* 39 */", 1},
        {"MATCH ()-[r]-(:Film) RETURN count(r) /* 11 */", 1},
        {"MATCH (:Person)-[r]-(:Film) RETURN count(r) /* 11 */", 1},
        // TODO(botu.wzy)
        // {"MATCH (:Person)-[r]-(:NO_LABEL) RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r:MARRIED]-() RETURN count(r) /* 8 */", 1},
        // TODO(botu.wzy)
        // {"MATCH ()-[r:NO_LABEL]-() RETURN count(r) /* 0 */", 1},
        {"MATCH ()-[r:MARRIED|BORN_IN]-() RETURN count(r) /* 20 */", 1},
        // TODO(botu.wzy)
        // {"MATCH ()-[r:MARRIED|BORN_IN|NO_LABEL]-() RETURN count(r) /* 20 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]-() RETURN count(r) /* 14 */", 1},
        {"MATCH (:City)-[r:MARRIED|BORN_IN]-() RETURN count(r) /* 6 */", 1},
        {"MATCH ()-[r:MARRIED|BORN_IN]-(:Person) RETURN count(r) /* 14 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]-(:City) RETURN count(r) /* 6 */", 1},
        {"MATCH (:Person)-[r:MARRIED|BORN_IN]-(:Film) RETURN count(r) /* 0 */", 1},
        {"MATCH (:Person)-[r:DIRECTED]-(:Film) RETURN count(r) /* 1 */", 1},
        {"MATCH (:Person)-[r:DIRECTED|ACTED_IN]-(:Film) RETURN count(r) /* 9 */", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

int test_fix_crash_issues(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        // issue #3: crash when WITH node/edge's alias
        {"MATCH (n:Person {name:'Liam Neeson'}) WITH n AS aa RETURN aa.name", 1},
        {"MATCH (n:Person {name:'Liam Neeson'})-[e]->(m) WITH e AS aa RETURN aa", 2},
        // issue #2: crash when SET multiple nodes
        // SET n.birthyear SET m.birthyear would return exeception
        {"MATCH (n:Person {name:'Liam Neeson'}) SET n.birthyear=2052 RETURN n.birthyear", 1},
        {"MATCH (n:Person {name:'Liam Neeson'}),(m:Person {name:'Richard Harris'}) "
         "SET n.birthyear=2152 RETURN n.birthyear,m.birthyear /*2152,1930*/",
         1},
        {"MATCH (n:Person {name:'Liam Neeson'}),(m:Person {name:'Richard Harris'}) "
         "SET n.birthyear=2252,m.birthyear=2230 RETURN n.birthyear,m.birthyear /*2252,2230*/",
         1},
        // issue #1-2: merge create edge with wrong direction
        {"MERGE (z3:Person {name:'zhang3'}) ON CREATE SET z3.birthyear=2021 ON MATCH SET "
         "z3.birthyear=2022\n"
         "WITH z3\n"
         "MERGE (z4:Person {name:'zhang4'}) ON CREATE SET z4.birthyear=2021 ON MATCH SET "
         "z4.birthyear=2022\n"
         "WITH z3,z4\n"
         "CREATE (z3)-[:HAS_CHILD]->(z4)",
         1},
        {"MATCH (z3:Person {name:'zhang3'})-[r]->(z4:Person {name:'zhang4'}) RETURN r", 1},
        {"MERGE (z3:Person {name:'zhang3'}) ON CREATE SET z3.birthyear=2021 ON MATCH SET "
         "z3.birthyear=2022\n"
         "WITH z3\n"
         "MERGE (z4:Person {name:'zhang4'}) ON CREATE SET z4.birthyear=2021 ON MATCH SET "
         "z4.birthyear=2022\n"
         "WITH z3,z4\n"
         "CREATE (z3)-[r:HAS_CHILD]->(z4) RETURN z3,z4,r",
         1},
        {"MATCH (z3:Person {name:'zhang3'})-[r]->(z4:Person {name:'zhang4'}) RETURN r", 2},
        // {"MERGE (z3:Person {name:'zhang3'}) ON CREATE SET z.birthyear=2021 ON MATCH SET
        // z.birthyear=2022/*expected exception*/"},
        // issue #98
        {"MATCH (m:City) RETURN collect(m.name) + [1,2]", 1},
        {"WITH [1,2] AS nn MATCH (m:City) RETURN collect(m.name) + nn", 1},
        {"MATCH (n:City) WITH collect(n.name) AS nn\n"
         "MATCH (m:City) RETURN collect(m.name) + nn",
         1},
        // issue #116
        {"MATCH (n:Person) RETURN -n.birthyear LIMIT 3", 3},
        {"MATCH (n:Person) RETURN -sum(n.birthyear) /*-27241*/", 1},
        // additional spaces in range literal
        {"MATCH (n) -[r:HAS_CHILD * 2 ]->(m) RETURN n,m ", 6},
        {"MATCH (n) -[r:HAS_CHILD * .. ]->(m) RETURN n,m ", 17},
        // issue #357, issue #148, github issue #188
        {"WITH '1' as s UNWIND ['a','b'] as k RETURN s,k", 2},
        {"WITH '1' as s UNWIND ['a','b']+s as k RETURN s,k", 3},
        {"MATCH (n:Person)-[]->(m:Film) WITH n.name AS nname, collect(id(m)) AS mc "
         "MATCH (n:Person {name: nname})<-[]-(o) WITH n.name AS nname, mc, collect(id(o)) AS oc "
         "UNWIND mc+oc AS c RETURN c",
         11},
        // github issue #322
        {"MATCH (m:Person)-[r:BORN_IN]->(n:City) "
         "WHERE n.name = 'London' and r.weight >= 1 and r.weight <= 100 RETURN sum(r.weight)",
         1}};
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    expected_exception_any(ctx, "MATCH (n:City) RETURN collect(n.name) + n.name");
    expected_exception_any(ctx, "MATCH (n:Person) RETURN NOT n.name");
    expected_exception_any(ctx, "MATCH (n:Person) RETURN -n.name");
    expected_exception_any(ctx, "REMOVE a.name");
    expected_exception_any(ctx, "SET a :MyLabel");
    expected_exception_any(ctx, "MATCH (n:Person) WITH n,n.name RETURN n.name");
    expected_exception_any(ctx, "WITH * MERGE(n:Person) RETURN n");
    expected_exception_any(ctx, "RETURN * UNION RETURN *");
    expected_exception_any(ctx, "RETURN * UNION RETURN 1 AS a");
    expected_exception_any(ctx, "RETURN 2 AS b UNION RETURN 1 AS a");
    expected_exception_any(ctx, "RETURN 2 AS b UNION RETURN 1 AS a, 3 AS c");
    expected_exception_any(ctx, "DELETE []");
    expected_exception_any(ctx, "DELETE [x in [1, 2, 3] | x]");
    expected_exception_any(ctx, "DELETE TRUE");
    // #issue 340
    expected_exception_any(ctx, "MERGE (n:null {id: 2909}) RETURN n");


    // issue #199
    expected_exception_any(ctx,
        "MATCH (n:Person {name:'Liam Neeson'}), "
        "(m:Person {name:'Liam Neeson'}), "
        "(o:Person {name:'Liam Neeson'}) "
        "WHERE custom.myadd('asd')='1' RETURN 1");
    // issue #312 & #463
    auto graph = ctx->graph_;
    ctx->graph_ = "";
    expected_exception_any(ctx, "MATCH (n) RETURN n LIMIT 5");
    expected_exception_any(ctx, "CALL db.vertexLabels");
    expected_exception_any(ctx, "CALL db.warmup");
    eval_script(ctx, "CALL dbms.graph.listGraphs()");
    expected_exception_any(ctx, "MATCH p=(n)-[e]->(m) RETURN p LIMIT 5");
    ctx->graph_ = graph;
    // issue #312 & #464 end
    // issue #473
    expected_exception_any(ctx, "MATCH (movie)<-[r]-(n) WITH n,n MATCH (n1) RETURN n1 LIMIT 1");
    expected_exception_any(ctx, "MATCH (movie)<-[r]-(n) return n,n limit 1");
    return 0;
}

int test_undefined_var(cypher::RTContext *ctx) {
    // RETURN undefined variable
    expected_exception_undefined_var(ctx, "MATCH (n:Person) RETURN q");
    expected_exception_undefined_var(ctx, "MATCH (n:Person) RETURN n.name + q.name");
    // WITH undefined variable
    expected_exception_undefined_var(ctx, "MATCH (n:Person) WITH q as m RETURN m");
    expected_exception_undefined_var(ctx, "MATCH (n:Person) WITH k RETURN n");
    // UNWIND undefined variable
    expected_exception_undefined_var(ctx, "UNWIND k as n RETURN n");
    // WHERE undefined variable
    expected_exception_undefined_var(ctx, "MATCH (n:Person) WHERE n.name = k RETURN n");
    // ORDERBY undefined variable
    expected_exception_undefined_var(ctx,
                                     "MATCH (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return "
                                     "v1.title,count(distinct v2) as cnt order by number");
    // List Comprehension undefined variable
    expected_exception_undefined_var(ctx, "WITH [1,2,3] AS k RETURN [x in k | x + y] AS result");
    // REMOVE undefined variable
    expected_exception_undefined_var(ctx, "REMOVE a.name");
    // SET undefined variable
    expected_exception_undefined_var(ctx, "SET a :MyLabel");
    expected_exception_undefined_var(ctx,
                                     "MATCH (n:Person {name:'D'}),(m:Person {name:'X'}) SET z=m");
    expected_exception_undefined_var(ctx,
                                     "MATCH (n:Person {name:'D'}),(m:Person {name:'X'}) SET n=z");
    expected_exception_undefined_var(ctx, "MATCH (n:Person {name:'X'}) SET z.name = 'Y'");
    // DELETE undefined variable
    expected_exception_undefined_var(ctx, "MATCH(n:Person {name: 'D'}) DELETE k");
    expected_exception_undefined_var(ctx, "MATCH(n:Person {name: 'D'}) DELETE k");
    expected_exception_undefined_var(ctx, "MATCH(n)-[r:REL]-(m) DELETE k");
    // WHERE undefined variable, test for realign alias id
    expected_exception_undefined_var(ctx,
                                     "MATCH (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) WITH v1, "
                                     "count(*) AS edge_num WHERE non_edge > 1 RETURN v1,edge_num");

    return 0;
}

/*
 * edge 中id和uid相关支持
 * 对齐rest api中语义
 * */
int test_edge_id_query(cypher::RTContext *ctx) {
    static const std::vector<std::pair<std::string, int>> script_check = {
        {"MATCH ()-[r]->() RETURN euid(r) /* 28 */", 28},
        {"MATCH ()-[r]->() RETURN id(r) /* 28 */", 28},
        // test rest api same support
        {"MATCH (n),(m) WHERE id(n)=0 and id(m)=1 CREATE (n)-[r:ACTED_IN {charactername: "
         "\"testaha\"}]->(m) RETURN r",
         1},
        {"MATCH (n),(m),(k) \n"
         "WHERE id(n)=0 and id(m)=2 and id(k)=3\n"
         "CREATE (n)-[r:ACTED_IN {charactername: \"testaha\"}]->(m),(n)-[q:MARRIED]->(k)\n"
         "RETURN r,q",
         1},
        {"MATCH (n)-[e]->() where id(n)=4 return euid(e)", 2},
        {"MATCH ()-[e]->(n) where id(n)=4 return euid(e)", 1},
        {"MATCH (n)-[e]-() where id(n)=4 return euid(e)", 3},
        {"MATCH ()-[e]->() where euid(e)=\"0_2_0_0_0\" return e,labels(e),properties(e)", 1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" return properties(e)", 1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" return e.charactername", 1},
        {"MATCH ()-[e]->() where euid(e)=\"8_13_2_0_0\" set e.weight=1223 return "
         "e,labels(e),properties(e)",
         1},
        {"MATCH ()-[e]->() where euid(e)=\"4_17_5_0_0\" delete e", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);
    return 0;
}

void TestCypherDetermineReadonly(cypher::RTContext *ctx) {
    std::string dummy;
    UT_EXPECT_EQ(
        cypher::Scheduler::DetermineReadOnly(
            ctx,
            lgraph_api::GraphQueryType::CYPHER,
            "match (n) return n limit 100",
            dummy,
            dummy),
        true);
    UT_EXPECT_EQ(
        cypher::Scheduler::DetermineReadOnly(
            ctx,
            lgraph_api::GraphQueryType::CYPHER,
            "CALL dbms.listGraphs()",
            dummy,
            dummy),
        true);
}

void TestCypherEmptyGraph(cypher::RTContext *ctx) {
    ctx->graph_ = "";
    expected_exception_any(ctx, "MATCH (n:Person) RETURN n.name LIMIT 1");
    ctx->graph_ = "default";
    eval_script(ctx, "MATCH (n:Person) RETURN n.name LIMIT 1");
}

/* the following query causes stack chaos:
 * "MATCH (n:Person) RETURN properties(n) LIMIT 2"  */
void debug_stack_chaos(cypher::RTContext *ctx) {
    auto access_ctr_db = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto txn = access_ctr_db.CreateReadTxn();
    auto it = txn.GetVertexIterator();
    std::string p;
    p.append("{");
    auto fields = txn.GetVertexFields(it);
    for (int i = 0; i < fields.size(); i++) {
        auto f = fields[i];
        p.append(f.first).append(":").append(f.second.ToString());
        if (i != fields.size() - 1) p.append(",");
    }
    p.append("}");

    UT_LOG() << p;
    txn.Abort();
}

int test_spatial_procedure(cypher::RTContext *ctx) {
    static const std::vector<std::string> scripts_ = {
    "CALL db.createVertexLabel('Location', 'name', 'name', 'string', false, 'geo', 'POINT', false)",
    "CREATE (a_:Location {name:'A_', geo: POINT(1.0, 2.0)})",
    "CREATE (b_:Location {name:'B_', geo: POINT(1.0, 2.0)})",
    "CALL db.createVertexLabel"
    "('Location_', 'name', 'name', 'string', false, 'geo1', 'LINESTRING', false,"
    "'geo2', 'POLYGON', false)",
    "CREATE (c_:Location_ "
    "{name:'C_', geo1: LINESTRING('0102000020E6100000030000000000000000000000000"
    "0000000000000000000000000004000000000000000400000000000000840000000000000F03F'), "
    "geo2: polygonwkt('POLYGON((0 0,0 7,4 2,2 0,0 0))')})"
    };
    eval_scripts(ctx, scripts_);

    static const std::vector<std::pair<std::string, int>> script_check = {
    {"with point(2.0, 2.0, 7203) as p1, point(2.0, 1.0, 7203) as p2\n"
     "CALL spatial.distance(p1, p2) YIELD distance RETURN distance = 1", 1},
    {"with LineStringWKB('01020000000300000000000000000000000000000000"
     "000000000000000000004000000000000000400000000000000840000000000000F03F') "
     "as linestring, "
     "PolygonWKT('POLYGON((0 0,0 7,4 2,2 0,0 0))', 4326) as polygon\n"
     "CALL spatial.distance(linestring, polygon) YIELD distance RETURN distance = 0", 1},
    {"MATCH (l1:Location {name:'A_'}), (l2:Location {name:'B_'}) with\n"
     "l1.geo as g1, l2.geo as g2\n"
     "CALL spatial.distance(g1, g2) YIELD distance RETURN distance = 0", 1},
    {"MATCH (l2:Location {name:'B_'}), (l3:Location_ {name:'C_'}) with\n"
     "l2.geo as g2, l3.geo2 as g3\n"
     "CALL spatial.distance(g2, g3) YIELD distance RETURN distance = 0", 1},
    {"MATCH (l1:Location_ {name:'C_'}) with\n"
     "l1.geo1 as g1, l1.geo2 as g2\n"
     "CALL spatial.distance(g1, g2) YIELD distance RETURN distance = 0", 1},
    };
    std::vector<std::string> scripts;
    std::vector<int> check;
    for (auto &s : script_check) {
        scripts.emplace_back(s.first);
        check.emplace_back(s.second);
    }
    eval_scripts_check(ctx, scripts, check);

    return 0;
}

enum TestCase {
    TC_FILE_SCRIPT = 1,
    TC_INTERACTIVE = 2,
    // read only
    TC_FIND = 3,
    TC_QUERY,
    TC_HINT,
    TC_MULTI_MATCH,
    TC_OPTIONAL_MATCH,
    TC_UNION,
    TC_FUNCTION,
    TC_PARAMETER,
    TC_VAR_LEN_EDGE,
    TC_UNIQUENESS,
    TC_FUNC_FILTER,
    TC_EXPRESSION,
    TC_WITH,
    TC_LIST_COMPREHENSION,
    TC_PATTERN_COMPREHENSION,
    TC_PROFILE,
    // write
    TC_UNWIND = 101,
    TC_PROCEDURE,
    TC_ADD,
    TC_SET,
    TC_DELETE,
    TC_REMOVE,
    TC_ORDER_BY,
    TC_MERGE,
    TC_CREATE_YAGO,
    TC_AGGREGATE,
    TC_ALGO,
    TC_TOPN,
    TC_SPATIAL_PROCEDURE,
    TC_ERROR_REPORT = 201,
    TC_DEBUG_STACK_CHAOS,
    TC_LDBC_SNB = 301,
    TC_OPT = 401,
    TC_FIX_CRASH_ISSUES = 402,
    TC_UNDEFINED_VAR = 403,
    TC_CREATE_LABEL = 404,
    TC_READONLY = 500,
    TC_EDGE_ID = 501,
    TC_INVALID_SCHEMA = 502,
    TC_EMPTY_GRAPH = 700,
};

struct ParamCypher {
    ParamCypher(int _tc, int _d) {
        tc = _tc;
        d = _d;
    }
    int tc;
    int d;
};

class TestCypher : public TuGraphTestWithParam<struct ParamCypher> {};

TEST_P(TestCypher, Cypher) {
    int test_case = 0;
    int database = 0;
    std::string file;
    auto str = fma_common::StringFormatter::Format(
        "Test case: {}-file script; {}-interactive; {}-find; {}-query; {}-hint;"
        " {}-multi-match; {}-optional-match; {}-union; {}-function; {}-parameter;"
        " {}-var-len-edge; {}-uniqueness; {}-function filter; {}-expression; {}-with; "
        "{}-list-comprehension"
        " {}-profile; {}-unwind; {}-procedure; {}-add; {}-set; {}-del; {}-remove; {}-order by; "
        "{}-merge;"
        " {}-create yago; {}-aggregate; {}-algo; {}-topn; {}-spatial_procedure; "
        "{}-error report; {}-snb; {}-optimization; {}-fix_crash_issues;"
        " {}-undefined_variable; {}-create_label; {}-determine_read_only; {}-edge_id_query;"
        " {}-empty_graph;",
        TC_FILE_SCRIPT, TC_INTERACTIVE, TC_FIND, TC_QUERY, TC_HINT, TC_MULTI_MATCH,
        TC_OPTIONAL_MATCH, TC_UNION, TC_FUNCTION, TC_PARAMETER, TC_VAR_LEN_EDGE, TC_UNIQUENESS,
        TC_FUNC_FILTER, TC_EXPRESSION, TC_WITH, TC_LIST_COMPREHENSION, TC_PROFILE, TC_UNWIND,
        TC_PROCEDURE, TC_ADD, TC_SET, TC_DELETE, TC_REMOVE, TC_ORDER_BY, TC_MERGE, TC_CREATE_YAGO,
        TC_AGGREGATE, TC_ALGO, TC_TOPN, TC_SPATIAL_PROCEDURE, TC_ERROR_REPORT, TC_LDBC_SNB, TC_OPT,
        TC_FIX_CRASH_ISSUES, TC_UNDEFINED_VAR, TC_CREATE_LABEL, TC_READONLY, TC_EDGE_ID,
        TC_EMPTY_GRAPH);
    test_case = GetParam().tc;
    database = GetParam().d;
    int argc = _ut_argc;
    char **argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(test_case, "tc", true).Comment(str);
    config.Add(database, "d", true)
        .Comment("Select database: 0-current, 1-new yago, 2-empty, 3-yago with constraints");
    config.Add(file, "f", true).Comment("File path");
    config.ExitAfterHelp();
    config.ParseAndFinalize(argc, argv);

    if (database == 0) {
        // just use existing ./test.db
    } else if (database == 1) {
        fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
        GraphFactory::create_yago("./testdb");
    } else if (database == 2) {
        fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
    } else {
        fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
        GraphFactory::create_yago_with_constraints("./testdb");
    }
    lgraph::Galaxy::Config gconf;
    gconf.dir = "./testdb";
    lgraph::Galaxy galaxy(gconf, true, nullptr);

    cypher::RTContext db(nullptr, &galaxy, lgraph::_detail::DEFAULT_ADMIN_NAME, "default");
    db.param_tab_ = g_param_tab;

    auto no_throw_test_case = [&] {
        switch (test_case) {
            case TC_FILE_SCRIPT:
                test_file_script(file, &db);
                break;
            case TC_INTERACTIVE:
                test_interactive(&db);
                break;
            case TC_FIND:
                test_find(&db);
                break;
            case TC_QUERY:
                test_query(&db);
                break;
            case TC_HINT:
                test_hint(&db);
                break;
            case TC_MULTI_MATCH:
                test_multi_match(&db);
                break;
            case TC_OPTIONAL_MATCH:
                test_optional_match(&db);
                break;
            case TC_UNION:
                test_union(&db);
                break;
            case TC_FUNCTION:
                test_function(&db);
                break;
            case TC_PARAMETER:
                test_parameter(&db);
                break;
            case TC_VAR_LEN_EDGE:
                test_var_len_expand(&db);
                break;
            case TC_UNIQUENESS:
                test_uniqueness(&db);
                break;
            case TC_FUNC_FILTER:
                test_func_filter(&db);
                break;
            case TC_EXPRESSION:
                test_expression(&db);
                break;
            case TC_WITH:
                test_with(&db);
                break;
            case TC_LIST_COMPREHENSION:
                test_list_comprehension(&db);
                break;
            case TC_PROFILE:
                test_profile(&db);
                break;
            case TC_UNWIND:
                test_unwind(&db);
                break;
            case TC_PROCEDURE:
                test_procedure(&db);
                break;
            case TC_ADD:
                test_add(&db);
                break;
            case TC_SET:
                test_set(&db);
                break;
            case TC_DELETE:
                test_delete(&db);
                break;
            case TC_REMOVE:
                test_remove(&db);
                break;
            case TC_ORDER_BY:
                test_order_by(&db);
                break;
            case TC_CREATE_YAGO:
                test_create_yago(&db);
                break;
            case TC_AGGREGATE:
                test_aggregate(&db);
                break;
            case TC_ALGO:
                test_algo(&db);
                break;
            case TC_TOPN:
                test_topn(&db);
                break;
            case TC_SPATIAL_PROCEDURE:
                test_spatial_procedure(&db);
                break;
            case TC_MERGE:
                test_merge(&db);
                break;
            case TC_ERROR_REPORT:
                test_error_report(&db);
                break;
            case TC_DEBUG_STACK_CHAOS:
                debug_stack_chaos(&db);
                break;
            case TC_LDBC_SNB:
                test_ldbc_snb(&db);
                break;
            case TC_OPT:
                test_opt(&db);
                break;
            case TC_FIX_CRASH_ISSUES:
                test_fix_crash_issues(&db);
                break;
            case TC_UNDEFINED_VAR:
                test_undefined_var(&db);
                break;
            case TC_CREATE_LABEL:
                test_create_label(&db);
                break;
            case TC_READONLY:
                TestCypherDetermineReadonly(&db);
                break;
            case TC_EDGE_ID:
                test_edge_id_query(&db);
                break;
            case TC_EMPTY_GRAPH:
                TestCypherEmptyGraph(&db);
                break;
            case TC_INVALID_SCHEMA:
                test_invalid_schema(&db);
                break;
            default:
                break;
        }
    };
    UT_EXPECT_NO_THROW(no_throw_test_case());
    fma_common::SleepS(1);  // Waiting for memory reclaiming by async task
}

using namespace ::testing;

INSTANTIATE_TEST_CASE_P(
    TestCypher, TestCypher,
    Values(ParamCypher{3, 1}, ParamCypher{4, 3}, ParamCypher{5, 1}, ParamCypher{6, 1},
           ParamCypher{7, 1}, ParamCypher{8, 1}, ParamCypher{9, 1}, ParamCypher{10, 1},
           ParamCypher{11, 1}, ParamCypher{12, 1}, ParamCypher{13, 1}, ParamCypher{14, 1},
           ParamCypher{15, 1}, ParamCypher{16, 1}, ParamCypher{18, 1}, ParamCypher{101, 1},
           ParamCypher{102, 1}, ParamCypher{103, 1}, ParamCypher{104, 2}, ParamCypher{105, 2},
           ParamCypher{106, 1}, ParamCypher{107, 1}, ParamCypher{108, 2}, ParamCypher{109, 2},
           ParamCypher{110, 2}, ParamCypher{111, 2}, ParamCypher{112, 1}, ParamCypher{113, 1},
           ParamCypher{301, 2}, ParamCypher{401, 1}, ParamCypher{402, 1}, ParamCypher{403, 1},
           ParamCypher{404, 2}, ParamCypher{500, 0}, ParamCypher{501, 1}, ParamCypher{502, 1}));
