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

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class ValidationPass {
 public:
    std::string name_;

    explicit ValidationPass(const std::string &name) : name_(name) {}

    virtual ~ValidationPass() {}

    /* This pass and all sub-passes are executed only if the function returns
     * true.  The default implementation returns true.  */
    virtual bool Gate() = 0;

    /* This is the code to run.  If this is not overridden, then there should
     * be sub-passes otherwise this pass does nothing.   */
    virtual int Execute() = 0;
};

}  // namespace cypher
