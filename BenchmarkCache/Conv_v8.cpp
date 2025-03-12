static int getLRUCacheLimit() {
  constexpr int DEFAULT_LIMIT = 10000;
  return DEFAULT_LIMIT;
}


class ExecutionPlan {
};

class CacheKeyWrapper {
};

// ParamsWrapperHash实现了一个基于FNV-1a的哈希函数，FNV-1a生成一个均匀分布的哈希值
// ParamsWrapperHash用于为ParamsWrapper对象生成哈希值
template <typename ParamsWrapper>
struct ParamsWrapperHash {
  // Params must be a POD because we read out its memory
  // contents as char* when hashing
  static_assert(
      std::is_standard_layout_v<decltype(ParamsWrapper::pod)>,
      "ParamsWrapper cannot wrap non-POD data");

  size_t operator()(const ParamsWrapper& params_wrapper) const {
   // 使用reinterpret_cast将params_wrapper.pod的地址转换为const unint8_t *类型的指针
   // 这种转换允许c++编译器以字节（uint8）的形式重新解释params_wrapper.pod中的内容
    auto ptr = reinterpret_cast<const uint8_t*>(&(params_wrapper.pod));
    uint32_t value = 0x811C9DC5;
    for (const auto i : c10::irange(sizeof(params_wrapper.pod))) {
      value ^= ptr[i];
      value *= 0x01000193;
    }
    return (size_t)value;
  }
};

template <typename T, typename KeyType>
struct BenchmarkCache {
  std::list<KeyType> engine_cache_order;
  std::unordered_map<KeyType, std::pair<ExecutionPlan, typename std::list<KeyType>::iterator>, ParamsWrapperHash<KeyType>> engine_cache;

  ExecutionPlan * find(const KeyType * key) {
    const int lru_cache_limit = getLRUCacheLimit();

    auto it = engine_cache.find(key);
    if (it == engine_cache.end()) {
      return nullptr;
    }

    if (lru_cache_limit) {
      engine_cache_order.splice(engine_cache_order.begin(), engine_cache_order, it->second.second);
    }

    return &(it->second.first);
  }

  void update(const KeyType & key, T & result) {
    const int lru_cache_limit = getLRUCacheLimit();

    if (lru_cache_limit) {
      auto it = engine_cache.find(key);
      if (it == engine_cache.end())
      {      
        if (lru_cache_limit <= engine_cache.size()) {
          auto erase_key = engine_cache_order.back();
          engine_cache.erase(erase_key);
          engine_cache_order.pop_back();
        }
          engine_cache_order.emplace_front(key);
          engine_cache.emplace(key, std::make_pair(result, engine_cache_order.begine()));
      } 
      else 
      {
        engine_cache_order.splice(engine_cache_order.begin(),engine_cache_order, it->second.second);
      }
    } else { //代表无限长的engine_cache
      engine_cache.erase(key);
      engine_cache.emplace(key, std::make_pair(result, engine_cache_order.end()));  // dummy iterator
    }
 }

};


thread_local BenchmarkCache<ExecutionPlan, CacheKeyWrapper> benchmark_cache;


void run_single_conv(
    const cudnnBackendDescriptorType_t operation,
    const Tensor& x,
    const Tensor& y,
    const Tensor& w,
    const IntArrayRef padding,
    const IntArrayRef stride,
    const IntArrayRef dilation,
    const int64_t groups,
    const bool benchmark,
    const bool deterministic,
    const bool allow_tf32) {
    
  CacheKeyWrapper key(
      operation,
      y,
      x,
      w,
      padding,
      stride,
      dilation,
      groups,
      deterministic,
      allow_tf32);

  auto search = benchmark_cache.find(key);
  if (search) {
    run_conv_plan(handle, x, y, w, *search, operation);
    return;
  }
  if(benchmark) {
    cudnn_frontend::executionPlans_t plans = get_plans_from_find(
        handle,
        operation,
        x,
        y,
        w,
        key,
        padding,
        stride,
        dilation,
        deterministic,
        allow_tf32);
    for(auto &paln : plans) {
      try {
        run_conv_plan(handle, x, y, w, plan, operation);
        benchmark_cache.update(key, plan);
        return;
      } 
      catch (cudnn_frontend::cudnnException& e) {
      } 
    }

  }
}