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

#pragma once

#include <string>
#include <atomic>
#include "http/http_server.h"

namespace lgraph {
namespace http {

// TODO(anyone): Merge this with Import Task
class AlgorithmTask {
 public:
    AlgorithmTask(HttpService* http_service, const std::string& taskId, const std::string& taskName,
                  const std::string& user, const std::string& token, const std::string& graphName,
                  const std::string& algoName, const std::string& algoType,
                  const std::string& version, const std::string& jobParam,
                  const std::string& nodeType, const std::string& edgeType,
                  const std::string& outputType, const double timeout, const bool inProcess);

    void operator()();

 private:
    HttpService* http_service_;
    std::string taskId_;
    std::string taskName_;
    std::string user_;
    std::string token_;
    std::string graphName_;
    std::string algoName_;
    std::string algoType_;
    std::string version_;
    std::string jobParam_;
    std::string nodeType_;
    std::string edgeType_;
    std::string outputType_;
    double timeout_;
    bool inProcess_;
};

}  //  end of namespace http
}  //  end of namespace lgraph
