#include <gtest/gtest.h>
#include "common/logger.h"

DEFINE_int32(num, 10000, "The number of inserted vertexes, the number of edges is ten times the number of points");
DEFINE_int32(thread, 16, "The number of threads that perform benchmark read and write operations");
DEFINE_int32(depth, 3, "Depth of k hop");

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e %t %l %s:%#] %v");
    testing::InitGoogleTest(&argc,argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    return RUN_ALL_TESTS();
}