#ifndef BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#define BEAM_APPLICATION_INTERRUPT_WIN32_HPP
#include <boost/thread.hpp>
#include <windows.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  inline bool& IsRunningWrapper() {
    static auto isRunning = true;
    return isRunning;
  }

  inline boost::mutex& RunningMutexWrapper() {
    static auto runningMutex = boost::mutex();
    return runningMutex;
  }

  inline BOOL __stdcall CtrlHandler(DWORD ctrl) {
    switch(ctrl) {
      case CTRL_C_EVENT:
        {
          auto lock = boost::lock_guard(RunningMutexWrapper());
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
    auto lock = boost::lock_guard(Details::RunningMutexWrapper());
    return Details::IsRunningWrapper();
  }

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool ReceivedKillEvent() {
    return !IsRunning();
  }

  /** Waits for a shutdown event. */
  inline void WaitForKillEvent() {
    while(!ReceivedKillEvent()) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
  }
}

#endif
