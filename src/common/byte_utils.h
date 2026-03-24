#pragma once

#include <cstring>
#include <string_view>
#include <type_traits>

namespace common {

template <typename T>
const char* AsChars(const T& value) {
  static_assert(std::is_trivially_copyable_v<T>);
  return reinterpret_cast<const char*>(&value);
}

template <typename T>
std::string_view AsStringView(const T& value) {
  static_assert(std::is_trivially_copyable_v<T>);
  return {AsChars(value), sizeof(T)};
}

template <typename T>
T ReadValue(const char* data) {
  static_assert(std::is_trivially_copyable_v<T>);
  T value;
  std::memcpy(&value, data, sizeof(T));
  return value;
}

}  // namespace common
