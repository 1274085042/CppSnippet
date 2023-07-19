#pragma once

#include "CompileTimeFunctionPointer.h"
#include <utility>

struct  OperatorKernel  
{
  virtual ~OperatorKernel() = default;
};

// template<class FuncPtr, class ReturnType, class ParameterList> 
// class WrapFunctionIntoFunctor_ 
// {};

template<class FuncPtr, class ReturnType, class... Parameters>
class WrapFunctionIntoFunctor_ final : public OperatorKernel 
{
    public:
      decltype(auto) operator()(Parameters... args) 
      {
        return (*FuncPtr::func_ptr())(std::forward<Parameters>(args)...);
      }
};