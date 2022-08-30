/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <string>
#include <functional>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace lgraph {
namespace dll {
#ifdef _WIN32
typedef HINSTANCE LibHandle;

inline LibHandle LoadDynamicLibrary(const std::string& path) { return LoadLibrary(path.c_str()); }

inline bool UnloadDynamicLibrary(LibHandle handle) { return FreeLibrary(handle); }

template <typename FuncT>
inline FuncT GetDllFunction(LibHandle handle, const std::string& func_name) {
    return (FuncT)GetProcAddress(handle, func_name.c_str());
}

inline std::string GetLastErrorMsg() {
    auto err = GetLastError();
    return "Error code: " + std::to_string(err);
}
#else
typedef void* LibHandle;

inline LibHandle LoadDynamicLibrary(const std::string& path) {
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
}

inline bool UnloadDynamicLibrary(LibHandle handle) { return (dlclose(handle) == 0); }

template <typename FuncT>
inline FuncT GetDllFunction(LibHandle handle, const std::string& func_name) {
    return (FuncT)dlsym(handle, func_name.c_str());
}

inline std::string GetLastErrorMsg() {
    char* c = dlerror();
    if (c) return std::string(c);
    return std::string();
}
#endif
}  // namespace dll
}  // namespace lgraph
