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

#include "compiler/gpc/ops/spmm.h"

using namespace gpc;
int main(int argc, char const *argv[]) {
    // Config inputs' dimentions
    int row = 232965;
    int nn = 128;
    int elems_perrow = 492; // row / 100 / 2;
    int nnz = row * elems_perrow;

    std::vector<int> params{row, nn, elems_perrow, nnz};

    int nthread = std::min(nn, 8);
    int mthread = 128 / nthread;
    switch (nn) {
    case 128:
        nthread = 8;
        mthread = 16;
        break;
    }
    // look, when m > 1, it's complicated for row balance pass, so we limit m =
    // 1 here.
    int mt = 1; // Don't change it
    int nt = nn / nthread;
    int kt = nthread;

    std::vector<int> config{mthread, nthread, mt, nt, kt};

    auto spmm =
        Spmm(true /*register tile*/, false /*reference*/, params, config);

    Buffer<float> ib(nn, row);
    for (int y = 0; y < ib.bottom(); y++) {
        for (int x = 0; x < ib.right(); x++) {
            ib(x, y) = 1.0f;
        }
    }
    Buffer<int> irowindices(row);
    for (int x = 0; x < irowindices.right(); x++) {
        irowindices(x) = x;
    }
    Buffer<int> irow(row + 1);
    for (int x = 0; x < irow.right(); x++) {
        irow(x) = x * elems_perrow;
    }
    Buffer<int> icol(nnz);
    for (int x = 0; x < icol.right(); x++) {
        icol(x) = x % elems_perrow;
    }
    Buffer<float> ob(nn, row);
    ob.fill(0.0f);

    // auto normal = Spmm(false);
    // printf("Speedup: %.2fX\n", normal.time / register_tile.time);
    return 0;
}