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
 *         lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_EXPR_EXPRNODEFWD_H_
#define GEAXFRONTEND_AST_EXPR_EXPRNODEFWD_H_

// This is the umbrella header for all clause headers

#include "geax-front-end/ast/expr/AggFunc.h"
#include "geax-front-end/ast/expr/AllDifferent.h"
#include "geax-front-end/ast/expr/BAdd.h"
#include "geax-front-end/ast/expr/BAggFunc.h"
#include "geax-front-end/ast/expr/BAnd.h"
#include "geax-front-end/ast/expr/BBitAnd.h"
#include "geax-front-end/ast/expr/BBitLeftShift.h"
#include "geax-front-end/ast/expr/BBitOr.h"
#include "geax-front-end/ast/expr/BBitRightShift.h"
#include "geax-front-end/ast/expr/BBitXor.h"
#include "geax-front-end/ast/expr/BConcat.h"
#include "geax-front-end/ast/expr/BDiv.h"
#include "geax-front-end/ast/expr/BEqual.h"
#include "geax-front-end/ast/expr/BGreaterThan.h"
#include "geax-front-end/ast/expr/BIn.h"
#include "geax-front-end/ast/expr/BIndex.h"
#include "geax-front-end/ast/expr/BLike.h"
#include "geax-front-end/ast/expr/BMod.h"
#include "geax-front-end/ast/expr/BMul.h"
#include "geax-front-end/ast/expr/BSquare.h"
#include "geax-front-end/ast/expr/BNotEqual.h"
#include "geax-front-end/ast/expr/BNotGreaterThan.h"
#include "geax-front-end/ast/expr/BNotSmallerThan.h"
#include "geax-front-end/ast/expr/BOr.h"
#include "geax-front-end/ast/expr/BSafeEqual.h"
#include "geax-front-end/ast/expr/BSmallerThan.h"
#include "geax-front-end/ast/expr/BSub.h"
#include "geax-front-end/ast/expr/BXor.h"
#include "geax-front-end/ast/expr/Case.h"
#include "geax-front-end/ast/expr/Cast.h"
#include "geax-front-end/ast/expr/Exists.h"
#include "geax-front-end/ast/expr/Function.h"
#include "geax-front-end/ast/expr/GetField.h"
#include "geax-front-end/ast/expr/If.h"
#include "geax-front-end/ast/expr/IsDestinationOf.h"
#include "geax-front-end/ast/expr/IsDirected.h"
#include "geax-front-end/ast/expr/IsLabeled.h"
#include "geax-front-end/ast/expr/IsNormalized.h"
#include "geax-front-end/ast/expr/IsNull.h"
#include "geax-front-end/ast/expr/IsSourceOf.h"
#include "geax-front-end/ast/expr/MatchCase.h"
#include "geax-front-end/ast/expr/MkList.h"
#include "geax-front-end/ast/expr/MkMap.h"
#include "geax-front-end/ast/expr/MkRecord.h"
#include "geax-front-end/ast/expr/MkSet.h"
#include "geax-front-end/ast/expr/MkTuple.h"
#include "geax-front-end/ast/expr/ListComprehension.h"
#include "geax-front-end/ast/expr/MultiCount.h"
#include "geax-front-end/ast/expr/Neg.h"
#include "geax-front-end/ast/expr/Not.h"
#include "geax-front-end/ast/expr/Param.h"
#include "geax-front-end/ast/expr/Ref.h"
#include "geax-front-end/ast/expr/Same.h"
#include "geax-front-end/ast/expr/Tilde.h"
#include "geax-front-end/ast/expr/TupleGet.h"
#include "geax-front-end/ast/expr/VBool.h"
#include "geax-front-end/ast/expr/VDate.h"
#include "geax-front-end/ast/expr/VDatetime.h"
#include "geax-front-end/ast/expr/VDouble.h"
#include "geax-front-end/ast/expr/VDuration.h"
#include "geax-front-end/ast/expr/VInt.h"
#include "geax-front-end/ast/expr/VNone.h"
#include "geax-front-end/ast/expr/VNull.h"
#include "geax-front-end/ast/expr/VSome.h"
#include "geax-front-end/ast/expr/VString.h"
#include "geax-front-end/ast/expr/VTime.h"
#include "geax-front-end/ast/expr/Windowing.h"

#endif  // GEAXFRONTEND_AST_EXPR_EXPRNODEFWD_H_
