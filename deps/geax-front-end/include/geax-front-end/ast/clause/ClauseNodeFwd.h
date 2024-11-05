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

#ifndef GEAXFRONTEND_AST_CLAUSE_CLAUSENODEFWD_H_
#define GEAXFRONTEND_AST_CLAUSE_CLAUSENODEFWD_H_

// This is the umbrella header for all clause headers
#include "geax-front-end/ast/clause/AllowAnonymousTable.h"
#include "geax-front-end/ast/clause/Edge.h"
#include "geax-front-end/ast/clause/EdgeOnJoin.h"
#include "geax-front-end/ast/clause/ElementPredicate.h"
#include "geax-front-end/ast/clause/Except.h"
#include "geax-front-end/ast/clause/GraphPattern.h"
#include "geax-front-end/ast/clause/Hint.h"
#include "geax-front-end/ast/clause/Intersect.h"
#include "geax-front-end/ast/clause/LabelAnd.h"
#include "geax-front-end/ast/clause/LabelNot.h"
#include "geax-front-end/ast/clause/LabelOr.h"
#include "geax-front-end/ast/clause/Node.h"
#include "geax-front-end/ast/clause/OpConcurrent.h"
#include "geax-front-end/ast/clause/OtherWise.h"
#include "geax-front-end/ast/clause/PathChain.h"
#include "geax-front-end/ast/clause/PathModePrefix.h"
#include "geax-front-end/ast/clause/PathPattern.h"
#include "geax-front-end/ast/clause/PathPrefix.h"
#include "geax-front-end/ast/clause/PathSearchPrefix.h"
#include "geax-front-end/ast/clause/PropStruct.h"
#include "geax-front-end/ast/clause/ReadConsistency.h"
#include "geax-front-end/ast/clause/ResetAll.h"
#include "geax-front-end/ast/clause/ResetGraph.h"
#include "geax-front-end/ast/clause/ResetParam.h"
#include "geax-front-end/ast/clause/ResetSchema.h"
#include "geax-front-end/ast/clause/ResetTimeZone.h"
#include "geax-front-end/ast/clause/SchemaFromPath.h"
#include "geax-front-end/ast/clause/SetAllProperties.h"
#include "geax-front-end/ast/clause/SetGraphClause.h"
#include "geax-front-end/ast/clause/SetLabel.h"
#include "geax-front-end/ast/clause/SetParamClause.h"
#include "geax-front-end/ast/clause/SetSchemaClause.h"
#include "geax-front-end/ast/clause/SetSingleProperty.h"
#include "geax-front-end/ast/clause/SetTimeZoneClause.h"
#include "geax-front-end/ast/clause/SingleLabel.h"
#include "geax-front-end/ast/clause/TableFunctionClause.h"
#include "geax-front-end/ast/clause/Union.h"
#include "geax-front-end/ast/clause/UpdateProperties.h"
#include "geax-front-end/ast/clause/WhereClause.h"
#include "geax-front-end/ast/clause/YieldField.h"
#include "geax-front-end/ast/clause/RemoveSingleProperty.h"

#endif  // GEAXFRONTEND_AST_CLAUSE_CLAUSENODEFWD_H_
