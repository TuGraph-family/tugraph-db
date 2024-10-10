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

#include <benchmark/benchmark.h>
#include "QueryTester.h"

// Define the external variables
GraphFactory::GRAPH_DATASET_TYPE _ut_graph_dataset_type = GraphFactory::GRAPH_DATASET_TYPE::YAGO;
lgraph::ut::QUERY_TYPE _ut_query_type = lgraph::ut::QUERY_TYPE::CYPHER;

// Function to run the core test code
void TestDemoFunction() {
    QueryTester tester;
    tester.RunTestDemo();
}

// Benchmark function
static void BM_TestDemoCol(benchmark::State& state) {
    for (auto _ : state) {
        // You can adjust the loop count as needed
        for (int i = 0; i < 10; ++i) {
            TestDemoFunction();
        }
    }
}

// Register the benchmark
BENCHMARK(BM_TestDemoCol);

int main(int argc, char** argv) {
    // Initialize logging if needed
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--dataset") {
            if (i + 1 < argc) {
                std::string dataset_type = argv[++i];  // Read dataset argument
                // Set _ut_graph_dataset_type based on dataset_type
                _ut_graph_dataset_type = GraphFactory::GRAPH_DATASET_TYPE::YAGO;
                // Add other dataset types as needed
            } else {
                std::cerr << "--dataset option requires one argument." << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "--query_type") {
            if (i + 1 < argc) {
                std::string query_type = argv[++i];  // Read query_type argument
                // Set _ut_query_type based on query_type
                if (query_type == "cypher") {
                    _ut_query_type = lgraph::ut::QUERY_TYPE::CYPHER;
                } else if (query_type == "gql") {
                    _ut_query_type = lgraph::ut::QUERY_TYPE::GQL;
                }
                // Add other query types as needed
            } else {
                std::cerr << "--query_type option requires one argument." << std::endl;
                return 1;
            }
        }
    }

    // Initialize Google Benchmark
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    return 0;
}
