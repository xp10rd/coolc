#pragma once

#include "codegen/arch/llvm/klass/KlassLLVM.h"
#include "codegen/decls/Runtime.h"
#include "decls/Decls.h"
#include <llvm/IR/Module.h>

namespace codegen
{
class RuntimeLLVM;

/**
 * @brief Information for one runtime method
 *
 */
struct RuntimeMethod
{
    llvm::Function *_func;

    /**
     * @brief Construct info for runtime method
     *
     * @param module LLVM Module to register method
     * @param name Method name
     * @param ret Return type
     * @param args Args types list
     * @param runtime Runtime info
     */
    RuntimeMethod(llvm::Module &module, const std::string &name, llvm::Type *ret,
                  const std::initializer_list<llvm::Type *> &args, RuntimeLLVM &runtime);
};

class RuntimeLLVM : public Runtime<RuntimeMethod, llvm::Value *>
{
    friend class RuntimeMethod;

  public:
    /**
     * @brief Void pointer type for convinience
     *
     */
    llvm::Type *_void_ptr_type;

    llvm::Type *_int32_type;

    llvm::Type *_void_type;

  private:
    const RuntimeMethod _equals;

    // Object class methods
    const RuntimeMethod _object_init;
    const RuntimeMethod _object_abort;
    const RuntimeMethod _object_type_name;
    const RuntimeMethod _object_copy;

    // String methods
    const RuntimeMethod _string_init;
    const RuntimeMethod _string_length;
    const RuntimeMethod _string_concat;
    const RuntimeMethod _string_substr;

    // IO methods
    const RuntimeMethod _io_init;
    const RuntimeMethod _io_out_string;
    const RuntimeMethod _io_out_int;
    const RuntimeMethod _io_in_string;
    const RuntimeMethod _io_in_int;

    // Int methods
    const RuntimeMethod _int_init;

    // Bool methods
    const RuntimeMethod _bool_init;

    // GC
    const RuntimeMethod _gc_alloc;
    const RuntimeMethod _gc_alloc_by_tag;

  public:
    /**
     * @brief Construct a new Runtime object
     *
     * @param module llvm::Module for function declarations
     */
    explicit RuntimeLLVM(llvm::Module &module);
};
}; // namespace codegen