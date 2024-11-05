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

#include "geax/logical/LogicalOperator.h"
#include "geax/logical/LogicalOperatorVisitor.h"
#include "geax/logical/core/LogicalAllNodesScan.h"
#include "geax/logical/core/LogicalNodeByLabelsScan.h"
#include "geax/logical/core/LogicalNodeSeek.h"
#include "geax/logical/core/LogicalExpandAll.h"
#include "geax/logical/core/LogicalVarLengthExpand.h"
#include "geax/logical/core/LogicalCreate.h"
#include "geax/logical/core/LogicalDelete.h"
#include "geax/logical/core/LogicalSetProperty.h"
#include "geax/logical/core/LogicalApply.h"
#include "geax/logical/core/LogicalSemiApply.h"
#include "geax/logical/core/LogicalAntiSemiApply.h"
#include "geax/logical/core/LogicalArgument.h"
#include "geax/logical/core/LogicalProcedureCall.h"
#include "geax/logical/core/LogicalCartesianProduct.h"
#include "geax/logical/core/LogicalProduceResults.h"
#include "geax/logical/core/LogicalOptional.h"
#include "geax/logical/core/LogicalFilter.h"
#include "geax/logical/core/LogicalProjection.h"
#include "geax/logical/core/LogicalSort.h"
#include "geax/logical/core/LogicalLimit.h"
#include "geax/logical/core/LogicalSkip.h"
#include "geax/logical/core/LogicalDistinct.h"
#include "geax/logical/core/LogicalAggregation.h"
#include "geax/logical/extensions/LogicalJoin.h"
#include "geax/logical/extensions/LogicalSemiJoin.h"
#include "geax/logical/extensions/LogicalAntiJoin.h"
#include "geax/logical/extensions/LogicalUnion.h"
#include "geax/logical/extensions/LogicalIntersect.h"
#include "geax/logical/extensions/LogicalMinus.h"
#include "geax/logical/extensions/LogicalGetEdges.h"
#include "geax/logical/extensions/LogicalExtractEdge.h"
