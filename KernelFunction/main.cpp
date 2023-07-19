#include "KernelFuntion_impl.h"
#include "make_boxed_from_unboxed_functor.h"
#include <iostream>

using namespace std;
namespace at
{
    int add(int &a, int &b)
    {
        int res = a+b;
        cout<<">>>>>>>>>>>> "<< res << endl;
        return res;
    }
}
//template struct CompileTimeFunctionPointer<int(int&, int&), at::add>;
using CTFP = CompileTimeFunctionPointer<int(int &, int &), at::add>;

OperatorKernel * ok = new WrapFunctionIntoFunctor_<CTFP, int, int &, int &>();
using WFIF = WrapFunctionIntoFunctor_<CTFP, int, int &, int &>;

//template struct wrap_kernel_functor_unboxed_<WFIF, int(int&, int&)>;
using WKFU = wrap_kernel_functor_unboxed_< WFIF, int(int &, int  &)>;

void * unboxed_kernel_func = reinterpret_cast<void*> (WKFU::call);

KernelFunction kf{ok, unboxed_kernel_func};

int main()
{
    int a = 1;
    int b = 2;
    kf.call<int>(a, b);
}