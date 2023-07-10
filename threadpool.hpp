#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "details/function_wrapper.hpp"
#include "details/work_queue.hpp"

namespace lyc {

class thread_pool {
 public:
 // TODO(ye): Add a constructor to specify the number of threads
  thread_pool() : done_(false) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.emplace_back([this]() { worker_thread(); });
      }
    } catch (...) {
      // TODO(ye): need to change thread join with RAII style
      done_ = true;
      throw;
    }
  }
  ~thread_pool() {
    for (auto& t : threads_) {
      t.join();
    }
    done_ = true;
  }
  template <typename FunctionType, typename... Args>
  auto spawn_task(FunctionType&& f, Args&&... a) {
    using result_type = std::invoke_result_t<FunctionType&&, Args&&...>;
    std::packaged_task<result_type(Args...)> task(
        std::forward<FunctionType>(f));
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->emplace(std::move(task), std::forward<Args>(a)...);
    } else {
      pool_work_queue_.push(
          function_wrapper(std::move(task), std::forward<Args>(a)...));
    }
    return res;
  }

 private:
  void worker_thread() {
    local_work_queue_ = std::make_unique<local_queue_type>();
    while (!done_) {
      if (local_work_queue_ && !local_work_queue_->empty()) {
        auto task = std::move(local_work_queue_->front());
        local_work_queue_->pop();
        task();
      } else if (auto task = pool_work_queue_.pop(); task != nullptr) {
        (*task)();
      } else {
        std::this_thread::yield();
      }
    }
  }

 private:
  using local_queue_type = std::queue<function_wrapper>;
  std::vector<std::thread> threads_;
  std::atomic<bool> done_;
  lock_free_queue<function_wrapper> pool_work_queue_;
  static thread_local std::unique_ptr<local_queue_type> local_work_queue_;
};

thread_local std::unique_ptr<thread_pool::local_queue_type> thread_pool::local_work_queue_{nullptr};
}  // namespace lyc
#endif  // THREAD_POOL_HPP_