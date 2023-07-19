#pragma once

template<class KernelFunctor, class OpSignature>
struct wrap_kernel_functor_unboxed_ final 
{};
template<class KernelFunctor, class ReturnType, class... ParameterTypes>
struct wrap_kernel_functor_unboxed_<KernelFunctor, ReturnType(ParameterTypes...)> final 
{
    // See [Note: Argument forwarding in the dispatcher] for why ParameterTypes doesn't use &&
    static ReturnType call(OperatorKernel* functor, ParameterTypes... args) 
    {
      KernelFunctor* functor_ = static_cast<KernelFunctor*>(functor);
      // We're explicitly taking in a dispatchKeySet and forwarding it to the registered kernel.
      // See Note [Plumbing Keys Through The Dispatcher 2] for details.
      return (*functor_)(std::forward<ParameterTypes>(args)...);
    }
};