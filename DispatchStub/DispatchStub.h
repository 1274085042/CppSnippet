
#include <atomic>

enum class DeviceType : int8_t 
{
  CPU = 0,
  CUDA = 1, // CUDA.
  MKLDNN = 2, // Reserved for explicit MKLDNN
  OPENGL = 3, // OpenGL
  OPENCL = 4, // OpenCL
  IDEEP = 5, // IDEEP.
  HIP = 6, // AMD HIP
  FPGA = 7, // FPGA
  ORT = 8, // ONNX Runtime / Microsoft
  XLA = 9, // XLA / TPU
  Vulkan = 10, // Vulkan
  Metal = 11, // Metal
  XPU = 12, // XPU
  MPS = 13, // MPS
  Meta = 14, // Meta (tensors with no data)
  HPU = 15, // HPU / HABANA
  VE = 16, // SX-Aurora / NEC
  Lazy = 17, // Lazy Tensors
  IPU = 18, // Graphcore IPU
  PrivateUse1 = 19, // PrivateUse1 device
  // NB: If you add more devices:
  //  - Change the implementations of DeviceTypeName and isValidDeviceType
  //    in DeviceType.cpp
  //  - Change the number below
  COMPILE_TIME_MAX_DEVICE_TYPES = 20,
};

/**
 * The sole purpose of this class is to outline methods that don't need to be
 * specialized or otherwise inlined and duplicated (by the compiler due to
 * template expansion), since it causes size bloat if there are a significant
 * number of specialization of the DispatchStub<> class.
 */
struct  DispatchStubImpl 
{
  void* get_call_ptr( DeviceType device_type )
  {
    switch (device_type) 
    {
    case DeviceType::CPU:
      return cpu_dispatch_ptr;

    default:
        break;
    }
  }

    void* cpu_dispatch_ptr = nullptr;
    void* hip_dispatch_ptr = nullptr;
    void* mps_dispatch_ptr = nullptr;
};

template <typename FnPtr>
struct DispatchStub;

template <typename rT, typename... Args>
struct DispatchStub<rT (*)(Args...)> 
{
  using FnPtr = rT (*) (Args...);

  DispatchStub() = default;
  DispatchStub(const DispatchStub&) = delete;
  DispatchStub& operator=(const DispatchStub&) = delete;

private:
  FnPtr get_call_ptr(DeviceType device_type) 
  {
    return reinterpret_cast<FnPtr>( impl.get_call_ptr(device_type));
  }

public:
  template <typename... ArgTypes>
  rT operator()(DeviceType device_type, ArgTypes&&... args) {
    FnPtr call_ptr = get_call_ptr(device_type);
    return (*call_ptr)(std::forward<ArgTypes>(args)...);
  }

  void set_cpu_dispatch_ptr(FnPtr fn_ptr) 
  {
    impl.cpu_dispatch_ptr = reinterpret_cast<void*>(fn_ptr);
  }


private:
    DispatchStubImpl impl;
};


namespace 
{
    template <typename DispatchStub>
    struct RegisterCPUDispatch 
    {
        RegisterCPUDispatch(DispatchStub &stub, typename DispatchStub::FnPtr value) 
        {
        stub.set_cpu_dispatch_ptr(value);
        }
    };
} 

#define DECLARE_DISPATCH(fn, name)         \
  struct name : DispatchStub<fn> {         \
    name() = default;                      \
    name(const name&) = delete;            \
    name& operator=(const name&) = delete; \
  };                                       \
  extern  struct name name

#define DEFINE_DISPATCH(name) struct name name

#define REGISTER_CPU_DISPATCH(name, fn) \
  static RegisterCPUDispatch<struct name> name ## __register(name, fn);