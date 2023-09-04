/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Functions to provide smarter use of jemalloc, if jemalloc is being used.
// http://www.canonware.com/download/jemalloc/jemalloc-latest/doc/jemalloc.html

#ifndef GEAXFRONTEND_COMMON_FOLLYMALLOC_H_
#define GEAXFRONTEND_COMMON_FOLLYMALLOC_H_

#define FOLLY_ASSUME_NO_TCMALLOC

#if (defined(USE_JEMALLOC) || defined(FOLLY_USE_JEMALLOC)) && \
    !defined(FOLLY_SANITIZE)
#if defined(FOLLY_ASSUME_NO_JEMALLOC)
#error \
    "Both USE_JEMALLOC/FOLLY_USE_JEMALLOC and FOLLY_ASSUME_NO_JEMALLOC defined"
#endif
// JEMalloc provides it's own implementation of
// malloc_usable_size, and that's what we should be using.
#if defined(__FreeBSD__)
#include <malloc_np.h> // @manual
#else
#include <jemalloc/jemalloc.h> // @manual
#endif
#else
#if !defined(__FreeBSD__)
#if __has_include(<malloc.h>)
#include <malloc.h>
#endif
#endif

#if defined(__APPLE__) && !defined(FOLLY_HAVE_MALLOC_USABLE_SIZE)
// MacOS doesn't have malloc_usable_size()
extern "C" size_t malloc_usable_size(void* ptr);
#elif defined(_WIN32)
extern "C" size_t malloc_usable_size(void* ptr);
#endif
#endif

/**
 * Define various MALLOCX_* macros normally provided by jemalloc.  We define
 * them so that we don't have to include jemalloc.h, in case the program is
 * built without jemalloc support.
 */
#if (defined(USE_JEMALLOC) || defined(FOLLY_USE_JEMALLOC)) && \
    !defined(FOLLY_SANITIZE)
// We have JEMalloc, so use it.
#else
#ifndef MALLOCX_LG_ALIGN
#define MALLOCX_LG_ALIGN(la) (la)
#endif
#ifndef MALLOCX_ZERO
#define MALLOCX_ZERO (static_cast<int>(0x40))
#endif
#endif

#include "geax-front-end/common/FollyMallocImpl.h" /* nolint */

#include <cstddef>

namespace folly::clone {


/**
 * Determine if we are using jemalloc or not.
 */
#if defined(FOLLY_ASSUME_NO_JEMALLOC) || defined(FOLLY_SANITIZE)
  inline bool usingJEMalloc() noexcept {
    return false;
  }
#elif defined(USE_JEMALLOC) && !defined(FOLLY_SANITIZE)
  inline bool usingJEMalloc() noexcept {
    return true;
  }
#else
inline bool usingJEMalloc() noexcept {
  // Checking for rallocx != nullptr is not sufficient; we may be in a
  // dlopen()ed module that depends on libjemalloc, so rallocx is resolved, but
  // the main program might be using a different memory allocator.
  // How do we determine that we're using jemalloc? In the hackiest
  // way possible. We allocate memory using malloc() and see if the
  // per-thread counter of allocated memory increases. This makes me
  // feel dirty inside. Also note that this requires jemalloc to have
  // been compiled with --enable-stats.
  static const bool result = []() noexcept {
    // Some platforms (*cough* OSX *cough*) require weak symbol checks to be
    // in the form if (mallctl != nullptr). Not if (mallctl) or if (!mallctl)
    // (!!). http://goo.gl/xpmctm
    if (mallocx == nullptr || rallocx == nullptr || xallocx == nullptr ||
        sallocx == nullptr || dallocx == nullptr || sdallocx == nullptr ||
        nallocx == nullptr || mallctl == nullptr ||
        mallctlnametomib == nullptr || mallctlbymib == nullptr) {
      return false;
    }

    // "volatile" because gcc optimizes out the reads from *counter, because
    // it "knows" malloc doesn't modify global state...
    /* nolint */ volatile uint64_t* counter;
    size_t counterLen = sizeof(uint64_t*);

    if (mallctl(
            "thread.allocatedp",
            static_cast<void*>(&counter),
            &counterLen,
            nullptr,
            0) != 0) {
      return false;
    }

    if (counterLen != sizeof(uint64_t*)) {
      return false;
    }

    uint64_t origAllocated = *counter;

    static void* volatile ptr = malloc(1);
    if (!ptr) {
      // wtf, failing to allocate 1 byte
      return false;
    }

    free(ptr);

    return (origAllocated != *counter);
  }
  ();

  return result;
}
#endif

#if defined(FOLLY_ASSUME_NO_TCMALLOC) || defined(FOLLY_SANITIZE)
  inline bool usingTCMalloc() noexcept {
    return false;
  }
#elif defined(USE_TCMALLOC) && !defined(FOLLY_SANITIZE)
  inline bool usingTCMalloc() noexcept {
    return true;
  }
#else
inline bool usingTCMalloc() noexcept {
  static const bool result = []() noexcept {
    // Some platforms (*cough* OSX *cough*) require weak symbol checks to be
    // in the form if (mallctl != nullptr). Not if (mallctl) or if (!mallctl)
    // (!!). http://goo.gl/xpmctm
    if (MallocExtension_Internal_GetNumericProperty == nullptr ||
        sdallocx == nullptr || nallocx == nullptr) {
      return false;
    }
    static const char kAllocBytes[] = "generic.current_allocated_bytes";

    size_t before_bytes = 0;
    getTCMallocNumericProperty(kAllocBytes, &before_bytes);

    static void* volatile ptr = malloc(1);
    if (!ptr) {
      // wtf, failing to allocate 1 byte
      return false;
    }

    size_t after_bytes = 0;
    getTCMallocNumericProperty(kAllocBytes, &after_bytes);

    free(ptr);

    return (before_bytes != after_bytes);
  }
  ();

  return result;
}
#endif

inline bool canSdallocx() noexcept {
  static bool rv = usingJEMalloc() || usingTCMalloc();
  return rv;
}

inline void sizedFree(void* ptr, size_t size) {
  if (canSdallocx()) {
    sdallocx(ptr, size, 0);
  } else {
    free(ptr);
  }
}

}

#endif  // GEAXFRONTEND_COMMON_FOLLYMALLOC_H_
