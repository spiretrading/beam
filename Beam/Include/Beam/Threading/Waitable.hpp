#ifndef BEAM_WAITABLE_HPP
#define BEAM_WAITABLE_HPP
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class Waitable
      \brief Base class for an object that blocks on a condition.
   */
  class Waitable {
    public:

      //! Constructs a Waitable object.
      Waitable() = default;

      virtual ~Waitable() = default;

    protected:

      //! Returns <code>true</code> iff the object is available.
      /*!
        \param waitable The object to test for availability.
      */
      static bool IsAvailable(const Waitable& waitable);

      //! Returns <code>true</code> iff the object is available.
      virtual bool IsAvailable() const = 0;

      //! Waits for the object to be available.
      /*!
        \param lock The lock synchronizing access to this object.
      */
      void Wait(boost::unique_lock<boost::mutex>& lock) const;

      //! Returns the mutex synchronizing access to this object.
      boost::mutex& GetMutex() const;

      //! Notifies one Routine that this object is available.
      void NotifyOne();

      //! Notifies all Routines that this object is available.
      void NotifyAll();

    private:
      mutable boost::mutex m_mutex;
      mutable ConditionVariable m_isAvailableCondition;
  };

  inline bool Waitable::IsAvailable(const Waitable& waitable) {
    return waitable.IsAvailable();
  }

  inline boost::mutex& Waitable::GetMutex() const {
    return m_mutex;
  }

  inline void Waitable::Wait(boost::unique_lock<boost::mutex>& lock) const {
    while(!IsAvailable()) {
      m_isAvailableCondition.wait(lock);
    }
  }

  inline void Waitable::NotifyOne() {
    m_isAvailableCondition.notify_one();
  }

  inline void Waitable::NotifyAll() {
    m_isAvailableCondition.notify_all();
  }
}
}

#endif
