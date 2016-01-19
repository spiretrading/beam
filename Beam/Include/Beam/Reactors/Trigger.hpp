#ifndef BEAM_TRIGGER_HPP
#define BEAM_TRIGGER_HPP
#include <deque>
#include <functional>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/Control.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"

namespace Beam {
namespace Reactors {

  /*! \class Trigger
      \brief Invokes a callback from within a ReactorMonitor, for the purpose of
             synchronizing an external action with a ReactorMonitor.
   */
  class Trigger : private boost::noncopyable {
    public:

      //! The type of callback to invoke.
      using Callback = std::function<void ()>;

      //! Constructs a Trigger.
      /*!
        \param monitor The ReactorMonitor to synchronize with.
      */
      Trigger(ReactorMonitor& monitor);

      ~Trigger();

      //! Passes a callback to invoke asynchronously from within this Trigger's
      //! ReactorMonitor.
      /*!
        \param callback The Callback to invoke.
      */
      void Do(Callback callback);

    private:
      mutable boost::mutex m_mutex;
      MultiQueueWriter<bool> m_publisher;
      std::shared_ptr<PublisherReactor<MultiQueueWriter<bool>*>> m_trigger;
      std::deque<Callback> m_callbacks;
  };

  inline Trigger::Trigger(ReactorMonitor& monitor)
      : m_trigger(MakePublisherReactor(&m_publisher)) {
    monitor.AddEvent(m_trigger);
    monitor.AddReactor(Reactors::Do(
      [=] (bool trigger) {
        Callback callback;
        {
          boost::lock_guard<boost::mutex> lock(m_mutex);
          callback = std::move(m_callbacks.front());
          m_callbacks.pop_front();
        }
        callback();
      }, m_trigger));
  }

  inline Trigger::~Trigger() {
    m_publisher.Break();
  }

  inline void Trigger::Do(Callback callback) {
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      m_callbacks.push_back(std::move(callback));
    }
    m_publisher.Push(true);
  }
}
}

#endif
