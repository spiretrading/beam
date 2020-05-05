#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#define DOCTEST_MAIN_TIMEOUT(timeout)                                          \
  int main(int argc, char** argv) {                                            \
    auto is_done = std::atomic_bool(false);                                    \
    auto is_done_condition = std::condition_variable();                        \
    auto is_done_mutex = std::mutex();                                         \
    auto lock = std::unique_lock(is_done_mutex);                               \
    auto thread = new std::thread(                                             \
      [&] {                                                                    \
        auto code = doctest::Context(argc, argv).run();                        \
        auto lock = std::unique_lock(is_done_mutex);                           \
        is_done = true;                                                        \
        is_done_condition.notify_one();                                        \
      });                                                                      \
    if(!is_done) {                                                             \
      if(argc >= 2 && std::string(argv[1]) == "--notimeout") {                 \
        is_done_condition.wait(lock);                                          \
      } else {                                                                 \
        is_done_condition.wait_for(lock, std::chrono::seconds(timeout));       \
      }                                                                        \
    }                                                                          \
    if(!is_done) {                                                             \
      std::cout << "Status: TIMEOUT (" << argv[0] << ")" << std::endl;         \
      std::quick_exit(0);                                                      \
    }                                                                          \
    return 0;                                                                  \
  }

#define DOCTEST_MAIN() DOCTEST_MAIN_TIMEOUT(10)
