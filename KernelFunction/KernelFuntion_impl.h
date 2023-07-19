#pragma once

#include "WrapFunctionIntoFunctor.h"
#include "KernelFuntion.h"
#include <memory>

inline KernelFunction::KernelFunction(OperatorKernel* boxed_kernel_func, void* unboxed_kernel_func)
  : boxed_kernel_func_(boxed_kernel_func)
  , unboxed_kernel_func_(unboxed_kernel_func)
{}

template<class Return, class... Args>
inline Return callUnboxedKernelFunction(void* unboxed_kernel_func, OperatorKernel* functor,Args&&... args) 
{
    using ActualSignature = Return (OperatorKernel*, Args...);
    ActualSignature* func = reinterpret_cast<ActualSignature*>(unboxed_kernel_func);
    return (*func)(functor, std::forward<Args>(args)...);
};

template<class Return, class ... Args>
Return KernelFunction::call( Args &&... args) const 
{
    return callUnboxedKernelFunction<Return, Args...>( unboxed_kernel_func_, boxed_kernel_func_, std::forward<Args>(args)...);
}
