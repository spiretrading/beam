#ifndef BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#define BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <thread>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

namespace Beam {
namespace Details {
  inline std::atomic<bool>& get_running_flag() noexcept {
    static auto flag = std::atomic<bool>(true);
    return flag;
  }

  inline std::mutex& get_shutdown_mutex() noexcept {
    static auto mutex = std::mutex();
    return mutex;
  }

  inline std::condition_variable& get_shutdown_condition() noexcept {
    static auto condition = std::condition_variable();
    return condition;
  }

  inline int* get_signal_pipe() noexcept {
    static auto fds = std::array<int, 2>{-1, -1};
    static auto initialized = [] {
      if(pipe(fds.data()) == 0) {
        fcntl(fds[0], F_SETFL, O_NONBLOCK);
        fcntl(fds[1], F_SETFL, O_NONBLOCK);
      }
      return true;
    }();
    (void)initialized;
    return fds.data();
  }

  inline void signal_handler(int signal) noexcept {
    get_running_flag().store(false, std::memory_order_release);
    auto pipe_fds = get_signal_pipe();
    if(pipe_fds[1] != -1) {
      auto byte = char(1);
      (void)::write(pipe_fds[1], &byte, 1);
    }
  }

  inline bool install_signal_handlers() noexcept {
    auto pipe_fds = get_signal_pipe();
    if(pipe_fds[0] == -1 || pipe_fds[1] == -1) {
      return false;
    }
    struct ::sigaction action {};
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    auto result = true;
    result &= (sigaction(SIGINT, &action, nullptr) == 0);
    result &= (sigaction(SIGTERM, &action, nullptr) == 0);
    result &= (sigaction(SIGHUP, &action, nullptr) == 0);
    result &= (sigaction(SIGQUIT, &action, nullptr) == 0);
    return result;
  }
}

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool is_running() noexcept {
    static auto handlers_installed = Details::install_signal_handlers();
    if(!handlers_installed) {
      return false;
    }
    return Details::get_running_flag().load(std::memory_order_acquire);
  }

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool received_kill_event() noexcept {
    return !is_running();
  }

  /** Waits for a shutdown event. */
  inline void wait_for_kill_event() noexcept {
    auto pipe_fds = Details::get_signal_pipe();
    if(pipe_fds[0] == -1) {
      while(Details::get_running_flag().load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      return;
    }
    while(Details::get_running_flag().load(std::memory_order_acquire)) {
      auto read_fds = fd_set();
      FD_ZERO(&read_fds);
      FD_SET(pipe_fds[0], &read_fds);
      auto timeout = timeval(1, 0);
      auto result =
        select(pipe_fds[0] + 1, &read_fds, nullptr, nullptr, &timeout);
      if(result > 0) {
        auto buffer = std::array<char, 64>();
        while(::read(pipe_fds[0], buffer.data(), sizeof(buffer)) > 0) {}
      }
    }
  }
}

#endif
