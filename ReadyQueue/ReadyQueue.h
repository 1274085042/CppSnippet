#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <iostream>
#include <condition_variable>

struct NodeTask 
{
  int input_;
  // When worker receives a task with isShutdownTask = true, it will immediately
  // exit. The engine sends a shutdown task to every queue upon its destruction.
  bool isShutdownTask_;
  NodeTask(int input, bool isShutdownTask):input_(input), isShutdownTask_(isShutdownTask)
  {}

  void dump()
  {
    std::cerr<<input_<<" : "<<isShutdownTask_<<std::endl;
  }
};


struct ReadyQueue 
{
 private:
  struct CompareNodeTaskTime 
  {
    bool operator()(NodeTask const& t1, NodeTask const& t2) 
    {
      if (t1.input_ < t2.input_) 
      {
        return true;
      } 
      else 
      {
        return false;
      }
    }
  };

  // To notify threads waiting on the ReadyQueue of available tasks on the heap_
  std::condition_variable not_empty_;
  // To protect read and writes to heap_
  mutable std::mutex mutex_;

  std::priority_queue<NodeTask, std::vector<NodeTask>, CompareNodeTaskTime> heap_;

 public:
  void dump()
  {
    for (; !heap_.empty(); heap_.pop())
    {
        std::cout<<heap_.top().input_<<std::endl;
    }
  }
  // incrementOutstandingTasks indicates whether or not we should increment
  // 'outstanding_tasks_' for the associated GraphTask. This should mostly
  // always be true and is only set false in certain cases (see docs for
  // DistEngine.execute_graph_task_until_ready_queue_empty)
  void push(NodeTask item)
  {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      heap_.push(std::move(item));
    }
    not_empty_.notify_one();
  }

  void pushShutdownTask()
  {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      heap_.push(NodeTask(-1, true));
    }
    not_empty_.notify_one();
  }

  NodeTask pop()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this](){return !heap_.empty();});
    auto task = std::move(const_cast<NodeTask &>(heap_.top()));
    heap_.pop();
    return task;
  }

  bool empty() const
  {
    return heap_.empty();
  }

  size_t size() const
  {
    // Lock mutex for accesses to heap_
    std::unique_lock<std::mutex> lock(mutex_);
    return heap_.size();
  }
};

static thread_local std::shared_ptr<ReadyQueue> tls_local_ready_queue;

struct  Engine 
{
  std::vector<std::shared_ptr<ReadyQueue>> device_ready_queues_;
  std::once_flag start_device_threads_flag_;
  int num_devices_ = 3;
  
  void init_local_ready_queue(std::shared_ptr<ReadyQueue> ready_queue = nullptr)
  {
    if (ready_queue)
    {
        tls_local_ready_queue = std::move(ready_queue);
    } 
    else if (!tls_local_ready_queue)
    {
        tls_local_ready_queue = std::make_shared<ReadyQueue>();
    }

  }
  
  void thread_init(int device, const std::shared_ptr<ReadyQueue>& ready_queue)
  {
      init_local_ready_queue(ready_queue);
      NodeTask task = tls_local_ready_queue.get()->pop();
      if (task.isShutdownTask_) 
      {
        return;
      }
      else
      {
        std::cout<<"thread: "<<std::this_thread::get_id()<<std::endl;
      }
  }

  void start_device_threads()
  {
    device_ready_queues_ = std::vector<std::shared_ptr<ReadyQueue>>(num_devices_);
    for (auto& queue : device_ready_queues_) 
    {
      queue = std::make_shared<ReadyQueue>();
    }
    for (int i=0; i<num_devices_; ++i) 
    {
      std::thread t(&Engine::thread_init, this, i, device_ready_queues_[i]);
      t.detach();
    }
  }

  void initialize_device_threads_pool() 
  {

    std::call_once(
      start_device_threads_flag_, &Engine::start_device_threads, this);
  }

  void execute_with_node_task()
  {
    initialize_device_threads_pool();
    // TODO:select device 1
    auto queue = device_ready_queues_[1];
    queue->push(NodeTask(1, false));
  }

  void execute()
  {
    init_local_ready_queue();
    execute_with_node_task();
  }

  void stop()
  {
    for(auto & queue : device_ready_queues_)
    {
        queue->pushShutdownTask();
    }
  }

  ~Engine()
  {
    stop();
  }
};
