/**
 * Copyright 2024 AntGroup CO., Ltd.
 *
 * Copyright (c) "Neo4j"
 * Neo4j Sweden AB [https://neo4j.com]
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

/*
 * written by botu.wzy, inspired by Neo4j Go Driver
 */
#pragma once
#include <string>

namespace bolt {
// Point2D represents a two dimensional point in a particular coordinate reference system.
struct Point2D {
    double x = 0;
    double y = 0;
    uint32_t spatialRefId = 0;  // id of coordinate reference system.
};

// Point3D represents a three dimensional point in a particular coordinate reference system.
struct Point3D {
    double x = 0;
    double y = 0;
    double z = 0;
    uint32_t spatialRefId = 0;  // id of coordinate reference system.
};

}  // namespace bolt
