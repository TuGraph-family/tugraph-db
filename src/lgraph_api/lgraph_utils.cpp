/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <sys/time.h>
#include <string>
#include <stdexcept>
#include "lgraph/lgraph_utils.h"

namespace lgraph_api {

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1e6);
}

void* alloc_buffer(size_t bytes) {
    void* buffer;
#if USE_VALGRIND
    buffer = malloc(bytes);
    if (!buffer) {
        throw std::bad_alloc();
    }
#else
    buffer = mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (buffer == MAP_FAILED) {
        throw std::runtime_error("memory allocation failed");
    }
#endif
    return buffer;
}

void dealloc_buffer(void* buffer, size_t bytes) {
#if USE_VALGRIND
    free(buffer);
#else
    int error = munmap(buffer, bytes);
    if (error != 0) {
        fprintf(stderr, "warning: potential memory leak!\n");
    }
#endif
}
}  // namespace lgraph_api
