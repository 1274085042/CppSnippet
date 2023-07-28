#pragma once
#include "c10/Registry.h"


struct  CUDAHooksInterface 
{
    virtual ~CUDAHooksInterface() {}
    virtual void initCUDA() const {}

};

const CUDAHooksInterface& getCUDAHooks();

// struct  CUDAHooksArgs {};

// 1 声明一个typed registry函数，type指的是CUDAHooksInterface，
//   并别名一个Registerer -> RegistererCUDAHooksRegistry
C10_DECLARE_REGISTRY(CUDAHooksRegistry, CUDAHooksInterface);
/*
attribute__((__visibility__("default")))
Registry<std::string, std::unique_ptr<CUDAHooksInterface>, CUDAHooksArgs>* CUDAHooksRegistry(); 

typedef Registerer<std::string, std::unique_ptr<CUDAHooksInterface>, CUDAHooksArgs> RegistererCUDAHooksRegistry
*/

#define REGISTER_CUDA_HOOKS(clsname) \
  C10_REGISTER_CLASS(CUDAHooksRegistry, clsname, clsname)