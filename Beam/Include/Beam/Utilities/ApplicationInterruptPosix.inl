#ifndef BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#define BEAM_APPLICATION_INTERRUPT_POSIX_HPP
#include <signal.h>
#include <boost/thread.hpp>
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

  inline void CtrlHandler(int sig) {
    auto lock = boost::lock_guard(RunningMutexWrapper());
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
