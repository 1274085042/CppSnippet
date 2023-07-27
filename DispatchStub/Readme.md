自定义的函数注册到DispatchStub对象中，并根据DeviceType选择注册的函数执行  


在Copy.h中
```
using fn_type = void(*)(const Tensor& x);
DECLARE_DISPATCH(fn_type, copy_stub);
```  
1 DECLARE_DISPATCH：负责copy_stub的声明， 继承自DispatchStub    

在Copy.cpp中  
```
DEFINE_DISPATCH(copy_stub);
```  
2 定义一个全局变量copy_stub  


3 在功能.cpp中（CopyKernel.cpp）将自定义的函数注册到copy_stub全局变量中     
```
REGISTER_CPU_DISPATCH(copy_stub , copy_kernel);  
```  

DispatchStub已经重载了oprator()，因此其对象是一个函数对象  
调用注册的函数：  
```
stub(DeviceType::CPU, i, b);
```

