#ifndef BEAM_TASKSIGNALHANDLER_HPP
#define BEAM_TASKSIGNALHANDLER_HPP
#include <boost/noncopyable.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/SignalHandling/QueuedSignalHandler.hpp"
#include "Beam/Threading/ThreadPool.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class TaskSignalHandler
      \brief Allows for the handling of signals in a concurrent task.
   */
  class TaskSignalHandler : private boost::noncopyable {
    public:

      //! Constructs a TaskSignalHandler.
      /*!
        \param threadPool The ThreadPool to handle the tasks in.
      */
      TaskSignalHandler(Ref<Threading::ThreadPool> threadPool);

      ~TaskSignalHandler();

      //! Returns a slot compatible with this signal handler.
      /*!
        \tparam SlotType A function type of the form R (P0, P1, ...)
        \param slot The slot to make compatible with this signal handler.
        \return A slot compatible with this signal handler.
      */
      template<typename SlotType>
      std::function<typename GetSignature<SlotType>::type> GetSlot(
        const SlotType& slot);

    private:
      boost::mutex m_mutex;
      Threading::ThreadPool* m_threadPool;
      QueuedSignalHandler m_queuedSignalHandler;
      std::function<void ()> m_signalProcessor;
      int m_taskCount;
      bool m_stopping;
      boost::condition_variable m_resultsEmptyCondition;

      void OnSignalsQueued();
      void OnSignalsProcessed();
  };

  inline TaskSignalHandler::TaskSignalHandler(
      Ref<Threading::ThreadPool> threadPool)
      : m_threadPool(threadPool.Get()),
        m_signalProcessor(std::bind(&QueuedSignalHandler::HandleSignals,
          std::ref(m_queuedSignalHandler))),
        m_taskCount(0),
        m_stopping(false) {
    m_queuedSignalHandler.SetQueuedSlot(
      std::bind(&TaskSignalHandler::OnSignalsQueued, this));
  }

  inline TaskSignalHandler::~TaskSignalHandler() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    while(m_taskCount != 0) {
      m_stopping = true;
      m_resultsEmptyCondition.wait(lock);
    }
  }

  inline void TaskSignalHandler::OnSignalsQueued() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    m_threadPool->Queue(m_signalProcessor, Routines::Eval<void>());
    ++m_taskCount;
  }

  inline void TaskSignalHandler::OnSignalsProcessed() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    --m_taskCount;
    if(m_stopping && m_taskCount == 0) {
      m_resultsEmptyCondition.notify_one();
    }
  }

  template<typename SlotType>
  std::function<typename GetSignature<SlotType>::type>
      TaskSignalHandler::GetSlot(const SlotType& slot) {
    return m_queuedSignalHandler.GetSlot<SlotType>(slot);
  }
}
}

#endif
