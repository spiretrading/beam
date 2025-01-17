#ifndef BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#define BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#include <mutex>
#include <thread>
#include <windows.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  inline bool& IsRunningWrapper() {
    static auto isRunning = true;
    return isRunning;
  }

  inline std::mutex& RunningMutexWrapper() {
    static auto runningMutex = std::mutex();
    return runningMutex;
  }

  inline BOOL __stdcall CtrlHandler(DWORD ctrl) {
    switch(ctrl) {
      case CTRL_C_EVENT:
        {
          auto lock = std::lock_guard(RunningMutexWrapper());
          IsRunningWrapper() = false;
        }
        return true;
    }
    return false;
  }
}

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool IsRunning() {
    static auto initializeControlHandler = SetConsoleCtrlHandler(
      reinterpret_cast<PHANDLER_ROUTINE>(Details::CtrlHandler), TRUE);
    auto lock = std::lock_guard(Details::RunningMutexWrapper());
    return Details::IsRunningWrapper();
  }

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool ReceivedKillEvent() {
    return !IsRunning();
  }

  /** Waits for a shutdown event. */
  inline void WaitForKillEvent() {
    while(!ReceivedKillEvent()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

#endif
