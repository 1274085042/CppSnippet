# BenchmarkCache Directory
实现了一个基于LRU（Least Recently Used）策略的缓存机制  

# 主要类和函数   

## 函数
run_single_conv ： 用于执行卷积操作，首先在benchmark_cache中查找execution_plan，  
如果找到则执行；  
如果没有找到，且启用了基准测试，则生成新的execution_plan并更新缓存（benchmark_cache）  

## BenchmarkCache Class  
模板类，用于实现ExecutionPlan缓存   

### 数据成员  
- engine_cache_order ： 维护缓存项的顺序（最近使用的在链表的开头，最长时间没用的在链表的结尾）  
- engine_cache ：用于存储缓存项   

### 成员函数
- ExecutionPlan * find(const KeyType *key) ： 根据键查找缓存中的execution_plan  
- void update(const KeyType & key, T & result) ：更新缓存