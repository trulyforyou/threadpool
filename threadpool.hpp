#ifndef THREAD_POOL_HPP_
#define THREAD_POOL_HPP_

#include <future>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "details/function_wrapper.hpp"
#include "details/work_queue.hpp"

namespace lyc {

class thread_guard {
 public:
  explicit thread_guard(std::thread& t) : t_(t) {}
  ~thread_guard() {
    if (t_.joinable()) {
      t_.join();
    }
  }
  thread_guard(thread_guard&&) = delete;
  thread_guard& operator=(thread_guard&&) = delete;
  thread_guard(const thread_guard&) = delete;
  thread_guard& operator=(const thread_guard&) = delete;

 private:
  std::thread& t_;
};

class thread_pool {
 public:
  explicit thread_pool(
      unsigned thread_count = std::thread::hardware_concurrency())
      : done_(false) {
    threads_.reserve(thread_count);
    guards_.reserve(thread_count);
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.emplace_back([this]() { worker_thread(); });
        guards_.emplace_back(std::make_unique<thread_guard>(threads_.back()));
      }
    } catch (...) {
      done_.test_and_set();
      throw;
    }
  }
  ~thread_pool() { done_.test_and_set(); }
  template <typename Func, typename... Args>
  auto spawn_task(Func&& f, Args&&... a)
      -> std::future<std::invoke_result_t<Func&&, Args&&...>> {
    using result_type = std::invoke_result_t<Func&&, Args&&...>;
    std::packaged_task<result_type()> task(
        std::bind(std::forward<Func>(f), std::forward<Args>(a)...));
    std::future<result_type> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->emplace(std::move(task));
    } else {
      pool_work_queue_.push(function_wrapper(std::move(task)));
    }
    return res;
  }

 private:
  void worker_thread() {
    local_work_queue_ = std::make_unique<local_queue_type>();
    while (!done_.test()) {
      try {
        if (local_work_queue_ && !local_work_queue_->empty()) {
          auto task = std::move(local_work_queue_->front());
          local_work_queue_->pop();
          task();
        } else if (auto task = pool_work_queue_.pop(); task != nullptr) {
          (*task)();
        } else {
          std::this_thread::yield();
        }
      } catch (const std::exception& e) {
        std::cerr << "Exception in worker thread: " << e.what() << '\n';
      } catch (...) {
        std::cerr << "Unknown exception in worker thread\n";
      }
    }
  }

 private:
  using local_queue_type = std::queue<function_wrapper>;
  std::vector<std::thread> threads_;
  std::vector<std::unique_ptr<thread_guard>> guards_;
  std::atomic_flag done_;
  lock_free_queue<function_wrapper> pool_work_queue_;
  static thread_local std::unique_ptr<local_queue_type> local_work_queue_;
};

thread_local std::unique_ptr<thread_pool::local_queue_type>
    thread_pool::local_work_queue_{nullptr};
}  // namespace lyc
#endif  // THREAD_POOL_HPP_