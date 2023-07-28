#pragma once
#include <mutex>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#define C10_EXPORT __attribute__((__visibility__("default")))
#define C10_IMPORT C10_EXPORT

#define C10_CONCATENATE_IMPL(s1, s2) s1##s2
#define C10_CONCATENATE(s1, s2) C10_CONCATENATE_IMPL(s1, s2)
#define C10_ANONYMOUS_VARIABLE(str) C10_CONCATENATE(str, __LINE__)

enum RegistryPriority 
{
  REGISTRY_FALLBACK = 1,
  REGISTRY_DEFAULT = 2,
  REGISTRY_PREFERRED = 3,
};


std::string demangle(const char* name) 
{
  // TODO: demangling on Windows
  if (!name) {
    return "";
  } else {
    return name;
  }
}

/// Returns the printable name of the type.
template <typename T>
inline const char* demangle_type() 
{
#ifdef __GXX_RTTI
  static const auto& name = *(new std::string(demangle(typeid(T).name())));
  return name.c_str();
#else // __GXX_RTTI
  return "(RTTI disabled, cannot show name)";
#endif // __GXX_RTTI
}

template <typename KeyType>
inline std::string KeyStrRepr(const KeyType& /*key*/) {
  return "[key type printing not supported]";
}

template <>
inline std::string KeyStrRepr(const std::string& key) {
  return key;
}


/**
 * @brief A template class that allows one to register classes by keys.
 * 一个类模板，允许通过key注册class
 * The keys are usually a std::string specifying the name, but can be anything
 * that can be used in a std::map.
 * key是一个string字符串
 * You should most likely not use the Registry class explicitly, but use the
 * helper macros below to declare specific registries as well as registering
 * objects.
 * 你最好不要显式地使用Registry类，而是使用下面的辅助宏声明特定的注册表和注册对象
 */
template <class SrcType, class ObjectPtrType, class... Args>
class Registry 
{
 public:
  typedef std::function<ObjectPtrType(Args...)> Creator;

  Registry(const Registry &) = delete;
  Registry& operator=(const Registry &) = delete;

  Registry(bool warning = true)
      : registry_(), priority_(), terminate_(true), warning_(warning) {}

  void Register(const SrcType& key, Creator creator, const RegistryPriority priority = REGISTRY_DEFAULT) \
  {
    std::lock_guard<std::mutex> lock(register_mutex_);
    // The if statement below is essentially the same as the following line:
    // CHECK_EQ(registry_.count(key), 0) << "Key " << key
    //                                   << " registered twice.";
    // However, CHECK_EQ depends on google logging, and since registration is
    // carried out at static initialization time, we do not want to have an
    // explicit dependency on glog's initialization function.
    if (registry_.count(key) != 0) 
    {
      auto cur_priority = priority_[key];
      if (priority > cur_priority) 
      {
#ifdef DEBUG
        std::string warn_msg =
            "Overwriting already registered item for key " + KeyStrRepr(key);
        fprintf(stderr, "%s\n", warn_msg.c_str());
#endif
        registry_[key] = creator;
        priority_[key] = priority;
      } 
      else if (priority == cur_priority) 
      {
        std::string err_msg =
            "Key already registered with the same priority: " + KeyStrRepr(key);
        fprintf(stderr, "%s\n", err_msg.c_str());
        if (terminate_) 
        {
          std::exit(1);
        } 
        else 
        {
          throw std::runtime_error(err_msg);
        }
      } 
      else if (warning_) 
      {
        std::string warn_msg =
            "Higher priority item already registered, skipping registration of " +
            KeyStrRepr(key);
        fprintf(stderr, "%s\n", warn_msg.c_str());
      }
    } 
    else 
    {
      registry_[key] = creator;
      priority_[key] = priority;
    }
  }

  void Register(const SrcType& key, Creator creator, const std::string& help_msg, const RegistryPriority priority = REGISTRY_DEFAULT) 
  {
    Register(key, creator, priority);
    help_message_[key] = help_msg;
  }

  inline bool Has(const SrcType& key) 
  {
    return (registry_.count(key) != 0);
  }

  ObjectPtrType Create(const SrcType& key, Args... args) 
  {
    if (registry_.count(key) == 0) 
    {
      // Returns nullptr if the key is not registered.
      return nullptr;
    }
    return registry_[key](args...);
  }

  /**
   * Returns the keys currently registered as a std::vector.
   */
  std::vector<SrcType> Keys() const 
  {
    std::vector<SrcType> keys;
    for (const auto& it : registry_) 
    {
      keys.push_back(it.first);
    }
    return keys;
  }

  inline const std::unordered_map<SrcType, std::string>& HelpMessage() const 
  {
    return help_message_;
  }

  const char* HelpMessage(const SrcType& key) const 
  {
    auto it = help_message_.find(key);
    if (it == help_message_.end()) 
    {
      return nullptr;
    }
    return it->second.c_str();
  }

  // Used for testing, if terminate is unset, Registry throws instead of
  // calling std::exit
  void SetTerminate(bool terminate) 
  { 
    terminate_ = terminate;
  }

 private:
  std::unordered_map<SrcType, Creator> registry_;
  std::unordered_map<SrcType, RegistryPriority> priority_;
  bool terminate_;
  const bool warning_;
  std::unordered_map<SrcType, std::string> help_message_;
  std::mutex register_mutex_;

};


template <class SrcType, class ObjectPtrType, class... Args>
class Registerer 
{
 public:
  explicit Registerer(const SrcType& key, 
  Registry<SrcType, ObjectPtrType, Args...>* registry, 
  typename Registry<SrcType, ObjectPtrType, Args...>::Creator creator, 
  const std::string& help_msg = "") 
  {
    registry->Register(key, creator, help_msg);
  }

  template <class DerivedType>
  static ObjectPtrType DefaultCreator(Args... args) 
  {
    return ObjectPtrType(new DerivedType(args...));
  }
};


/**
 * C10_DECLARE_TYPED_REGISTRY is a macro that expands to a function
 * declaration, as well as creating a convenient typename for its corresponding
 * registerer.
 */
/*
C10_DECLARE_TYPED_REGISTRY可扩展为函数声明，并为它相应的registerer创建一个类型名
*/
// Note on C10_IMPORT and C10_EXPORT below: we need to explicitly mark DECLARE
// as import and DEFINE as export, because these registry macros will be used
// in downstream shared libraries as well, and one cannot use *_API - the API
// macro will be defined on a per-shared-library basis. Semantically, when one
// declares a typed registry it is always going to be IMPORT, and when one
// defines a registry (which should happen ONLY ONCE and ONLY IN SOURCE FILE),
// the instantiation unit is always going to be exported.
//
// The only unique condition is when in the same file one does DECLARE and
// DEFINE - in Windows compilers, this generates a warning that dllimport and
// dllexport are mixed, but the warning is fine and linker will be properly
// exporting the symbol. Same thing happens in the gflags flag declaration and
// definition caes.
/*
注意注意下面的C10_IMPORT和C10_EXPORT：我们需要显式地将DECLARE表示为import，
将DEFINE标记为export，因为这些注册宏也会在下游的共享库中使用，并且不能使用*_API - API宏
将在每个共享库的基础上定义。
从语义上讲，当声明一个typed registry，它总是被IMPORT，
当定义一个registry（应该只定义一次，且定义在源文件中），实例化单元总是会被export
*/
#define C10_DECLARE_TYPED_REGISTRY(RegistryName, SrcType, ObjectType, PtrType, ...) \
  C10_IMPORT Registry<SrcType, PtrType<ObjectType>, ##__VA_ARGS__>* RegistryName(); \
  typedef Registerer<SrcType, PtrType<ObjectType>, ##__VA_ARGS__> Registerer##RegistryName

// C10_DECLARE_REGISTRY and C10_DEFINE_REGISTRY are hard-wired to use
// std::string as the key type, because that is the most commonly used cases.
// C10_DECLARE_REGISTRY 和 C10_DEFINE_REGISTRY被硬链接为使用string作为key的类型
// 1 声明一个RegistryName函数
#define C10_DECLARE_REGISTRY(RegistryName, ObjectType, ...) \
  C10_DECLARE_TYPED_REGISTRY(RegistryName, std::string, ObjectType, std::unique_ptr, ##__VA_ARGS__)


#define C10_DEFINE_TYPED_REGISTRY(RegistryName, SrcType, ObjectType, PtrType, ...) \
  C10_EXPORT Registry<SrcType, PtrType<ObjectType>, ##__VA_ARGS__>*                \
  RegistryName() {                                                                 \
    static Registry<SrcType, PtrType<ObjectType>, ##__VA_ARGS__>*                  \
        registry = new Registry<SrcType, PtrType<ObjectType>, ##__VA_ARGS__>();    \
    return registry;                                                               \
  }
// 2 定义1中声明的RegistryName函数
#define C10_DEFINE_REGISTRY(RegistryName, ObjectType, ...)  \
  C10_DEFINE_TYPED_REGISTRY(RegistryName, std::string, ObjectType, std::unique_ptr, ##__VA_ARGS__)


#define C10_REGISTER_TYPED_CLASS(RegistryName, key, ...)                    \
  static Registerer##RegistryName C10_ANONYMOUS_VARIABLE(g_##RegistryName)  \
  (                                                                         \
      key,                                                                  \
      RegistryName(),                                                       \
      Registerer##RegistryName::DefaultCreator<__VA_ARGS__>,                \
      demangle_type<__VA_ARGS__>());

#define C10_REGISTER_CLASS(RegistryName, key, ...) \
  C10_REGISTER_TYPED_CLASS(RegistryName, #key, __VA_ARGS__)