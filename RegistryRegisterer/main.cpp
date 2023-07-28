#include "ATen/CUDAHooksInterface.h"

int main()
{
    getCUDAHooks().initCUDA();
}