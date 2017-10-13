#ifndef BEAM_TRIGGER_HPP
#define BEAM_TRIGGER_HPP
#include <atomic>
#include <boost/noncopyable.hpp>
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class Trigger
      \brief Used to indicate that a Reactor has an update.
   */
  class Trigger : private boost::noncopyable {
    public:

      //! Constructs a Trigger.
      Trigger();

      //! Signals an update.
      /*!
        \return The sequence number associated with the update.
      */
      int SignalUpdate();

    private:
      std::atomic_int m_nextSequenceNumber;
  };

  inline Trigger::Trigger()
      : m_nextSequenceNumber{0} {}

  inline int Trigger::SignalUpdate() {
    auto sequenceNumber = ++m_nextSequenceNumber;
    return sequenceNumber;
  }
}
}

#endif
