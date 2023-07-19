#pragma once

#include "WrapFunctionIntoFunctor.h"
class  KernelFunction final 
{
public:
    explicit KernelFunction(OperatorKernel* boxed_kernel_func, void* unboxed_kernel_func);

    template<class Return, class... Args>
    Return call(Args &&... args) const;

private:
    OperatorKernel* boxed_kernel_func_;
    void* unboxed_kernel_func_;
};
