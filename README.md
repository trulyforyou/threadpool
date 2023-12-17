# Thread Pool in C++

This project provides a thread pool implementation in C++. It's designed for high-performance, concurrent programming in C++.

## Features

- **Thread Pool**: The [`thread_pool`](threadpool.hpp) class provides a pool of worker threads that can execute tasks concurrently.
- **Lock-Free Queue**: The [`lock_free_queue`](details/work_queue.hpp) class is a queue data structure that supports concurrent enqueue and dequeue operations without the need for locks.
- **Function Wrapper**: The [`function_wrapper`](details/function_wrapper.hpp) class is a utility that allows storing any callable object.

## Getting Started

To use this project, include the relevant header files in your C++ code:

```cpp
#include "threadpool.hpp"
```
Then, you can create a thread_pool object and use it to execute tasks:
```cpp
lyc::thread_pool pool;
auto result = pool.spawn_task([]() { /* your task here */ });
```
## Testing
To run the tests, compile and run the sample.cc file:

```shell
g++ -std=c++17 sample.cc -o sample
./sample
```
## License
This project is licensed under the terms of the LICENSE file.

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request.