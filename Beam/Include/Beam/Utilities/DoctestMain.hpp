#include <chrono>
#include <condition_variable>
#include <iostream>
#include <thread>
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#define DOCTEST_MAIN_TIMEOUT(timeout)                                          \
  int main(int argc, char** argv) {                                            \
    auto is_done = false;                                                      \
    auto is_done_mutex = std::mutex();                                         \
    auto is_done_condition = std::condition_variable();                        \
    auto lock = std::unique_lock(is_done_mutex);                               \
    auto thread = std::thread([&] {                                            \
      auto code = doctest::Context(argc, argv).run();                          \
      auto lock = std::unique_lock(is_done_mutex);                             \
      is_done = true;                                                          \
      is_done_condition.notify_one();                                          \
    });                                                                        \
    if(argc >= 2 && std::string(argv[1]) == "--notimeout") {                   \
      is_done_condition.wait(lock, [&] { return is_done; });                   \
    } else if(!is_done_condition.wait_for(lock,                                \
        std::chrono::seconds(timeout), [&] { return is_done; })) {             \
      std::cout << "Status: TIMEOUT (" << argv[0] << ")" << std::endl;         \
      std::quick_exit(0);                                                      \
    }                                                                          \
    thread.join();                                                             \
    return 0;                                                                  \
  }

#define DOCTEST_MAIN() DOCTEST_MAIN_TIMEOUT(10)
