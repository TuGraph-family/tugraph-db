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
 *
 *  Author:
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#ifndef FRONTEND_ISOGQL_GQLRESOLVECTX_H_
#define FRONTEND_ISOGQL_GQLRESOLVECTX_H_

#include "geax-front-end/isogql/parser/GqlParser.h"
#include "geax-front-end/common/ObjectAllocator.h"

namespace geax {
namespace frontend {

class GQLResolveCtx {
public:
    explicit GQLResolveCtx(geax::common::ObjectArenaAllocator& objAlloc)
        : objAlloc_(objAlloc) {}

    geax::common::ObjectArenaAllocator& objAlloc() { return objAlloc_; }

private:
    geax::common::ObjectArenaAllocator& objAlloc_;
};  // class GQLResolveCtx

}  // namespace frontend
}  // namespace geax

#endif  // FRONTEND_ISOGQL_GQLRESOLVECTX_H_
