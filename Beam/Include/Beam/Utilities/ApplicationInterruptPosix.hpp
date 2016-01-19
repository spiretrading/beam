#ifndef BEAM_APPLICATIONINTERRUPTPOSIX_HPP
#define BEAM_APPLICATIONINTERRUPTPOSIX_HPP
#include <boost/thread.hpp>
#include <signal.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  inline bool& IsRunningWrapper() {
    static bool isRunning = true;
    return isRunning;
  }

  inline boost::mutex& RunningMutexWrapper() {
    static boost::mutex runningMutex;
    return runningMutex;
  }

  inline void CtrlHandler(int sig) {
    boost::lock_guard<boost::mutex> lock(RunningMutexWrapper());
    IsRunningWrapper() = false;
  }

  inline bool InstallHandler() {
    ::signal(SIGINT, Details::CtrlHandler);
    return true;
  }
}

  //! Checks if the operating system signaled to shutdown the application.
  inline bool IsRunning() {
    static bool initializeControlHandler = Details::InstallHandler();
    boost::lock_guard<boost::mutex> lock(Details::RunningMutexWrapper());
    return Details::IsRunningWrapper();
  }

  //! Checks if the operating system signaled to shutdown the application.
  inline bool ReceivedKillEvent() {
    return !IsRunning();
  }

  //! Waits for a shutdown event.
  inline void WaitForKillEvent() {
    while(!ReceivedKillEvent()) {
      boost::this_thread::sleep(boost::posix_time::seconds{1});
    }
  }
}

#endif
