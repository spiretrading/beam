#ifndef BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#define BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <windows.h>

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

  inline BOOL __stdcall control_handler(DWORD control) noexcept {
    switch(control) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_CLOSE_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
        get_running_flag().store(false, std::memory_order_release);
        get_shutdown_condition().notify_all();
        return TRUE;
    }
    return FALSE;
  }

  inline bool install_handler() noexcept {
    return SetConsoleCtrlHandler(control_handler, TRUE) != 0;
  }
}

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool is_running() noexcept {
    static auto handler_installed = Details::install_handler();
    if(!handler_installed) {
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
    auto lock = std::unique_lock(Details::get_shutdown_mutex());
    Details::get_shutdown_condition().wait(lock, [] {
      return !Details::get_running_flag().load(std::memory_order_acquire);
    });
  }
}

#endif
