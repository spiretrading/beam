#ifndef BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#define BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#include <condition_variable>
#include <mutex>
#include <windows.h>

namespace Beam {
namespace Details {
  inline auto is_running = true;
  inline auto is_running_mutex = std::mutex();
  inline auto is_running_condition = std::condition_variable();

  inline BOOL __stdcall ctrl_handler(DWORD ctrl) {
    switch(ctrl) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_CLOSE_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
        {
          auto lock = std::lock_guard(is_running_mutex);
          is_running = false;
          is_running_condition.notify_all();
        }
        return true;
    }
    return false;
  }
}

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool is_running() {
    static auto initialize_control_handler = SetConsoleCtrlHandler(
      reinterpret_cast<PHANDLER_ROUTINE>(Details::ctrl_handler), TRUE);
    auto lock = std::lock_guard(Details::is_running_mutex);
    return Details::is_running;
  }

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool received_kill_event() {
    return !is_running();
  }

  /** Waits for a shutdown event. */
  inline void wait_for_kill_event() {
    if(!is_running()) {
      return;
    }
    auto lock = std::unique_lock(Details::is_running_mutex);
    Details::is_running_condition.wait(lock, [] {
      return !Details::is_running;
    });
  }
}

#endif
