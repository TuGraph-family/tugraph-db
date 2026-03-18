#pragma once

#include <vsag/vsag.h>

#include <mutex>

namespace graphdb {

inline void EnsureVsagInitialized() {
  static std::once_flag once;
  std::call_once(once, []() { (void)vsag::init(); });
}

}  // namespace graphdb
