/**
 * Copyright 2022 AntGroup CO., Ltd.
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
#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace cypher {
namespace compilation {
class JITCompiler;
class JITSymbolResolver;
class JITModuleMemoryManager;

/** Custom JIT implementation inspired by CHJIT in clickhouse
 * Main use cases:
 * 1. Compiled functions in module.
 * 2. Release memory for compiled function.
 */
class TuJIT {
 public:
    TuJIT();

    ~TuJIT();

    struct CompileModule {
        // Size of compiled module code in bytes
        size_t size_;

        // Module identifier. Should not be changed by client
        uint64_t identifier_;

        // Vector of compiled functions. Should not be changed by client.
        // It is client responsibility to cast result function to right signature.
        // After call to deleteCompiledModule compiled functions from module become invalid.
        std::unordered_map<std::string, void*> function_name_to_symbol_;
    };

    // Compile module. In compile function client responsiblity is to fill module with necessary
    // IR code, then it will be compiled by TuJIT instance.
    // Return compiled module.
    CompileModule compileModule(std::function<void(llvm::Module &)> compile_funciton);

    // Delete compiled module. Pointers to functions from module become invalid after this call.
    // It is client reponsibility to be sure that there are no pointers to compiled module code.
    void deleteCompiledModule(const CompileModule& module_info);

    // Register external symbol for TuJIT instance to use, during linking.
    // It can be function, or global constant.
    // It is client responsibility to be sure that address of symbol
    // is valid during TuJIT instance lifetime.
    void registerExternalSymbol(const std::string& symbol_name, void* address);

    // Total compiled code size for module that are current valid.
    size_t getCompiledCodeSize() const {
        return compiled_code_size_.load(std::memory_order_relaxed);
    }

 private:
    std::unique_ptr<llvm::Module> createModulerForCompilation();

    CompileModule compileModule(std::unique_ptr<llvm::Module> module);

    std::string getMangleName(const std::string& name_to_mangle) const;

    void runOptimizationPassesOnModule(llvm::Module& module) const;

    static std::unique_ptr<llvm::TargetMachine> getTargetMachine_;

    llvm::LLVMContext context_;
    std::unique_ptr<llvm::TargetMachine> machine_;
    llvm::DataLayout layout_;
    std::unique_ptr<JITCompiler> compiler_;

    std::unordered_map<uint64_t, std::unique_ptr<JITModuleMemoryManager>>
        module_identifier_to_memory_manager_;
    uint64_t current_module_key_ = 0;
    std::atomic<size_t> compiled_code_size_ = 0;
    mutable std::mutex jit_lock_;
};
}  // namespace compilation
}  // namespace cypher
