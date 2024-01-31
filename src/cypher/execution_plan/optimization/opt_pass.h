/**
 * Copyright 2024 AntGroup CO., Ltd.
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

//
// Created by wt on 19-11-27.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class OptPass {
 public:
    std::string name_;

    explicit OptPass(const std::string &name) : name_(name) {}

    virtual ~OptPass() {}

    /* This pass and all sub-passes are executed only if the function returns
     * true.  The default implementation returns true.  */
    virtual bool Gate() = 0;

    /* This is the code to run.  If this is not overridden, then there should
     * be sub-passes otherwise this pass does nothing.   */
    virtual int Execute(OpBase *root) = 0;
};

}  // namespace cypher
