#include "CUDAHooksInterface.h"
// NB: We purposely leak the CUDA hooks object.  This is because under some
// situations, we may need to reference the CUDA hooks while running destructors
// of objects which were constructed *prior* to the first invocation of
// getCUDAHooks.  The example which precipitated this change was the fused
// kernel cache in the JIT.  The kernel cache is a global variable which caches
// both CPU and CUDA kernels; CUDA kernels must interact with CUDA hooks on
// destruction.  Because the kernel cache handles CPU kernels too, it can be
// constructed before we initialize CUDA; if it contains CUDA kernels at program
// destruction time, you will destruct the CUDA kernels after CUDA hooks has
// been unloaded.  In principle, we could have also fixed the kernel cache store
// CUDA kernels in a separate global variable, but this solution is much
// simpler.
//
// CUDAHooks doesn't actually contain any data, so leaking it is very benign;
// you're probably losing only a word (the vptr in the allocated object.)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
/*
* 故意泄露CUDA hooks对象，这是因为在某些情况下，在运行对象的析够时，需要引用CUDA hooks
* 这些对象是第一次调用getCUDAHooks之前构造的，促成这种变化的例子是JIT中的融合kernel cache。
* kernel cache是一个全局变量，它缓存了CPU和CUDA kernels，CUDA内核在销毁时必须和CUDA hooks
* 交互。因为kernel cache也处理cpu kernels，它可以在初始化CUDA之前构建；如果在程序销毁时，
* 包含CUDA内核，你将在unload CUDA hooks后销毁CUDA内核，原则上，我们可以将存储CUDA内核的
* kernel cache放在一个单独的全局变量中，但这种方案简单的多

* CUDAHooks没有存储实际的数据，因此泄露它是良性的，你可能会丢失一个word(allocated 对象中的vptr)
*/
static CUDAHooksInterface* cuda_hooks = nullptr;

const CUDAHooksInterface& getCUDAHooks() {
  // NB: The once_flag here implies that if you try to call any CUDA
  // functionality before libATen_cuda.so is loaded, CUDA is permanently
  // disabled for that copy of ATen.  In principle, we can relax this
  // restriction, but you might have to fix some code.  See getVariableHooks()
  // for an example where we relax this restriction (but if you try to avoid
  // needing a lock, be careful; it doesn't look like Registry.h is thread
  // safe...)
  static std::once_flag once;
  std::call_once(once, [] {
    // Move MLU Hooks before CUDA Hooks tp raise MLU hook priority.
    // For solving some api(like pin_memory) failed when servers contain
    // MLU devices and GPU devices.
    cuda_hooks = CUDAHooksRegistry()->Create("MLUHooks").release();
    if (!cuda_hooks) {
      cuda_hooks = CUDAHooksRegistry()->Create("CUDAHooks").release();
      if (!cuda_hooks) {
        cuda_hooks = new CUDAHooksInterface();
      }
    }
  });
  return *cuda_hooks;
}

// 2 定义typed registry函数，type指的是CUDAHooksInterface
// 定义一个静态Registry指针，并返回
C10_DEFINE_REGISTRY(CUDAHooksRegistry, CUDAHooksInterface)
/*
__attribute__((__visibility__("default"))) 
Registry<std::string, std::unique_ptr<CUDAHooksInterface>,CUDAHooksArgs>* 
CUDAHooksRegistry() 
{ 
  static Registry<std::string, std::unique_ptr<CUDAHooksInterface>,CUDAHooksArgs>* registry 
  = new Registry<std::string, std::unique_ptr<CUDAHooksInterface>,CUDAHooksArgs>(); 
  return registry; 
}
*/