#include <future>
#include <iostream>

#include "threadpool.hpp"
using namespace lyc;

// Function: test_threadpool
// Description: This function is used to test the thread pool functionality.
// This function tests the functionality of the thread pool.
void test_threadpool() {
  thread_pool pool;  // Create a thread pool object

  int j = 1;  // Initialize variable j to 1

  std::vector<std::future<int>> futures;  // Create a vector to store futures
  futures.reserve(10);                    // Reserve capacity for 10 futures

  // Spawn 10 tasks in the thread pool and store the futures in the vector
  for (int i = 0; i < 10; ++i) {
    futures.emplace_back(pool.spawn_task([](int& i) { return i; }, j));
  }

  // Print the results of the tasks
  for (auto& f : futures) {
    std::cout << f.get() << std::endl;  // Print the result of the task
  }
}

int main() {
  test_threadpool();
  return 0;
}