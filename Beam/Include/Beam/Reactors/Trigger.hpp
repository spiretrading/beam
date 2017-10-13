#ifndef BEAM_TRIGGER_HPP
#define BEAM_TRIGGER_HPP
#include <atomic>
#include <boost/noncopyable.hpp>
#include "Beam/Queues/MultiQueueWriter.hpp"
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
        \param sequenceNumber The sequence number associated with the update.
      */
      void SignalUpdate(Out<int> sequenceNumber);

      //! Returns the sequence number publisher.
      const Publisher<int>& GetSequenceNumberPublisher() const;

    private:
      std::atomic_int m_nextSequenceNumber;
      MultiQueueWriter<int> m_sequencePublisher;
  };

  inline Trigger::Trigger()
      : m_nextSequenceNumber{0} {}

  inline void Trigger::SignalUpdate(Out<int> sequenceNumber) {
    *sequenceNumber = ++m_nextSequenceNumber;
    m_sequencePublisher.Push(*sequenceNumber);
  }

  inline const Publisher<int>& Trigger::GetSequenceNumberPublisher() const {
    return m_sequencePublisher;
  }
}
}

#endif
