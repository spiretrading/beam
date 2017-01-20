#ifndef BEAM_TRIGGERSLOT_HPP
#define BEAM_TRIGGERSLOT_HPP
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class TriggerSlot
      \brief Used to wait or query for a generic signal.
   */
  class TriggerSlot {
    public:

      //! Constructs a TriggerSlot.
      TriggerSlot();

      //! Returns <code>true</code> iff this slot was invoked.
      bool IsTriggered() const;

      //! Waits until this slot is triggered.
      void Wait() const;

      //! Waits until this slot is triggered.
      /*!
        \param timeout The amount of time to wait.
        \return <code>true</code> iff this slot was invoked.
      */
      bool Wait(boost::posix_time::time_duration timeout) const;

      template<typename... Args>
      void operator ()(Args&&... args) const;

    private:
      struct Implementation {
        boost::mutex m_mutex;
        bool m_triggered;
        boost::condition_variable m_isTriggeredCondition;
      };
      std::shared_ptr<Implementation> m_implementation;
  };

  inline TriggerSlot::TriggerSlot()
      : m_implementation(new Implementation()) {
    m_implementation->m_triggered = false;
  }

  inline bool TriggerSlot::IsTriggered() const {
    boost::lock_guard<boost::mutex> lock(m_implementation->m_mutex);
    return m_implementation->m_triggered;
  }

  inline void TriggerSlot::Wait() const {
    boost::unique_lock<boost::mutex> lock(m_implementation->m_mutex);
    while(!m_implementation->m_triggered) {
      m_implementation->m_isTriggeredCondition.wait(lock);
    }
  }

  inline bool TriggerSlot::Wait(boost::posix_time::time_duration timeout)
      const {
    boost::unique_lock<boost::mutex> lock(m_implementation->m_mutex);
    if(m_implementation->m_triggered) {
      return true;
    }
    m_implementation->m_isTriggeredCondition.timed_wait(lock, timeout);
    return m_implementation->m_triggered;
  }

  template<typename... Args>
  void TriggerSlot::operator ()(Args&&... args) {
    boost::lock_guard<boost::mutex> lock(m_implementation->m_mutex);
    m_implementation->m_triggered = true;
    m_implementation->m_isTriggeredCondition.notify_all();
  }
}
}

#endif
