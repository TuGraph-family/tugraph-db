#include <iostream>
#include <gtest/gtest.h>
#include "common/logger.h"

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e %t %l %s:%#] %v");
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}