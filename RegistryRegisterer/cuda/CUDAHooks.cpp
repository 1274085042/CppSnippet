#include "CUDAHooks.h"

void CUDAHooks::initCUDA() const 
{
    std::cout<<">>>>>>>>>>>>>>>> init cuda"<<std::endl;
}

REGISTER_CUDA_HOOKS(CUDAHooks);
/*
static RegistererCUDAHooksRegistry g_CUDAHooksRegistry8
( "CUDAHooks", 
CUDAHooksRegistry(), 
RegistererCUDAHooksRegistry::DefaultCreator<CUDAHooks>, 
demangle_type<CUDAHooks>());
*/
