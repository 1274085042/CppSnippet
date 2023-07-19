#pragma once
template <class FuncType_, FuncType_* func_ptr_>
struct CompileTimeFunctionPointer final 
{
  using FuncType = FuncType_;

  static constexpr FuncType* func_ptr() 
  {
    return func_ptr_;
  }
};
