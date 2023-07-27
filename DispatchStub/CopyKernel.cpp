//#include "DispatchStub.h"
#include "Copy.h"
#include <iostream>

void copy_kernel(int& iter, bool /*non_blocking*/) 
{
    std::cout<<"copy_kernel"<<std::endl;
}

REGISTER_CPU_DISPATCH(copy_stub, copy_kernel)
/*
static RegisterCPUDispatch<struct copy_stub> copy_stub__register(copy_stub, copy_kernel);
*/