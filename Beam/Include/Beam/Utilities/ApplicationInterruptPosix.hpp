#ifndef BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#define BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#include <mutex>
#include <thread>
#include <signal.h>
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

  inline void CtrlHandler(int sig) {
    auto lock = std::lock_guard(RunningMutexWrapper());
    IsRunningWrapper() = false;
  }

  inline bool InstallHandler() {
    ::signal(SIGINT, Details::CtrlHandler);
    return true;
  }
}

  /** Checks if the operating system signaled to shutdown the application. */
  inline bool IsRunning() {
    static auto initializeControlHandler = Details::InstallHandler();
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
