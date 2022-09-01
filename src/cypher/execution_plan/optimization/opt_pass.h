/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 19-11-27.
//
#pragma once

#include "execution_plan/execution_plan.h"

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
    virtual int Execute(ExecutionPlan *plan) = 0;
};

}  // namespace cypher
