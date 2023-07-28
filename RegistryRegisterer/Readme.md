# Registry.h  
1 `#define C10_DECLARE_REGISTRY(RegistryName, ObjectType, ...)`  用于声明一个RegistryName函数，函数返回类型是`Registry<. ,std::unique_ptr<ObjectType>, .> * `。并别名一个`Registerer<. ,std::unique_ptr<ObjectType>, .>` -> `RegistererRegistryName`，因为在`C10_REGISTER_TYPED_CLASS`中会使用这个别名的类型。该宏用在.h中  

2 `#define C10_DEFINE_REGISTRY(RegistryName, ObjectType, ...)`用于定义上面声明的RegistryName函数，里面构造了一个静态`Registry<. ,std::unique_ptr<ObjectType>, .> * `指针，并返回。该宏用在1中.h对应的.cpp中   

3 `#define C10_REGISTER_CLASS(RegistryName, key, ...)`会通过`#define C10_REGISTER_TYPED_CLASS(RegistryName, key, ...)`构造一个静态`RegistererRegistryName`对象   


**Registry可以抽象为一张注册表，key是被注册类的名字；value是一个函数指针，负责构造被注册类的对象**  
注册表的形式 : `unordered_map<SrcType, Creator> registry_`      

1 2 两步创建了一个静态`Registry<. ,std::unique_ptr<ObjectType>, .>`对象，该对象可以看成是一个ObjectType的注册表，意味着被注册的类是ObjectType的派生类     

**Registerer可以抽象为一个注册器，作用是为注册表填写key-value**  
3 步构造一个静态`RegistererRegistryName`对象，静态对象的构造过程中会向2中创建的静态`Registry<. ,std::unique_ptr<ObjectType>, .>`对象（也就是注册表）注册构造派生类对象的函数

# CUDAHooksInterface.h  
`#define REGISTER_CUDA_HOOKS(clsname)`对`#C10_REGISTER_CLASS(CUDAHooksRegistry, clsname, clsname)` 又封装了一下，方便了下游库注册自己的类

# 编译  
`$ g++ CUDAHooksInterface.cpp -fPIC --shared -o -g libaten.so`    
`$ g++ CUDAHooks.cpp -fPIC --shared -o -g libcuda.so`   
`$ g++ main.cpp ATen/libaten.so -Wl,--no-as-needed cuda/libcuda.so -g`   
