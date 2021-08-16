#ifndef BEAM_TASK_SIGNAL_HANDLER_HPP
#define BEAM_TASK_SIGNAL_HANDLER_HPP
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/SignalHandling/QueuedSignalHandler.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam::SignalHandling {

  /** Allows for the handling of signals in a concurrent task. */
  class TaskSignalHandler {
    public:

      /** Constructs a TaskSignalHandler. */
      TaskSignalHandler();

      ~TaskSignalHandler();

      /**
       * Returns a slot compatible with this signal handler.
       * @param <Slot> A function type of the form R (P0, P1, ...)
       * @param slot The slot to make compatible with this signal handler.
       * @return A slot compatible with this signal handler.
       */
      template<typename Slot>
      std::function<typename GetSignature<Slot>::type> GetSlot(
        const Slot& slot);

    private:
      boost::mutex m_mutex;
      QueuedSignalHandler m_queuedSignalHandler;
      std::function<void ()> m_signalProcessor;
      int m_taskCount;
      bool m_stopping;
      boost::condition_variable m_resultsEmptyCondition;

      TaskSignalHandler(const TaskSignalHandler&) = delete;
      TaskSignalHandler& operator =(const TaskSignalHandler&) = delete;
      void OnSignalsQueued();
      void OnSignalsProcessed();
  };

  inline TaskSignalHandler::TaskSignalHandler()
      : m_signalProcessor(std::bind_front(&QueuedSignalHandler::HandleSignals,
          std::ref(m_queuedSignalHandler))),
        m_taskCount(0),
        m_stopping(false) {
    m_queuedSignalHandler.SetQueuedSlot(
      std::bind_front(&TaskSignalHandler::OnSignalsQueued, this));
  }

  inline TaskSignalHandler::~TaskSignalHandler() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_taskCount != 0) {
      m_stopping = true;
      m_resultsEmptyCondition.wait(lock);
    }
  }

  inline void TaskSignalHandler::OnSignalsQueued() {
    auto lock = boost::lock_guard(m_mutex);
    Threading::Park(m_signalProcessor);
    ++m_taskCount;
  }

  inline void TaskSignalHandler::OnSignalsProcessed() {
    auto lock = boost::lock_guard(m_mutex);
    --m_taskCount;
    if(m_stopping && m_taskCount == 0) {
      m_resultsEmptyCondition.notify_one();
    }
  }

  template<typename Slot>
  std::function<typename GetSignature<Slot>::type> TaskSignalHandler::GetSlot(
      const Slot& slot) {
    return m_queuedSignalHandler.GetSlot<Slot>(slot);
  }
}

#endif
