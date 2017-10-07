#ifndef BEAM_APPLICATIONINTERRUPTWIN32_HPP
#define BEAM_APPLICATIONINTERRUPTWIN32_HPP
#include <boost/thread.hpp>
#include <windows.h>
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

  inline BOOL __stdcall CtrlHandler(DWORD ctrl) {
    switch(ctrl) {
      case CTRL_C_EVENT:
        {
          boost::lock_guard<boost::mutex> lock(RunningMutexWrapper());
          IsRunningWrapper() = false;
        }
        return true;
    }
    return false;
  }
}

  //! Checks if the operating system signaled to shutdown the application.
  inline bool IsRunning() {
    static BOOL initializeControlHandler = SetConsoleCtrlHandler(
      reinterpret_cast<PHANDLER_ROUTINE>(Details::CtrlHandler), TRUE);
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
