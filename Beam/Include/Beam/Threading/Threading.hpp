#ifndef BEAM_THREADING_HPP
#define BEAM_THREADING_HPP

namespace Beam::Threading {
  template<typename M> class CallOnce;
  class ConditionVariable;
  class LiveTimer;
  template<typename L> class LockRelease;
  class Mutex;
  template<bool A, typename M> class OptionalLock;
  template<typename M> struct PreferredConditionVariable;
  class RecursiveMutex;
  class ServiceThreadPool;
  template<typename T, typename M> class Sync;
  class TaskRunner;
  class ThreadPool;
  class TimedConditionVariable;
  class TimeoutException;
  struct Timer;
  class TimerBox;
  class TriggerTimer;
}

#endif
