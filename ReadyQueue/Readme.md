```cpp
template<
    class T,
    class Container = std::vector<T>,
    class Compare = std::less<typename Container::value_type>
> class priority_queue;
```
对于Compare  
- 如果left < right返回true，则priority_queue中的排列是[right, left]
- 如果left < right返回false, 则priority_queue中的排列是[left, right]    

Compare需要重载operator()

```cpp
  struct Compare
  {
    bool operator()(NodeTask const& t1, NodeTask const& t2) 
    {
      if (t1 < t2) 
      {
        return true;
      } 
      else 
      {
        return false;
      }
    }
  };
```