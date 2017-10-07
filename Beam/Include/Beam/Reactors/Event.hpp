#ifndef BEAM_REACTORSEVENT_HPP
#define BEAM_REACTORSEVENT_HPP
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class Event
      \brief Used to signal a change in a Reactor.
   */
  class Event : private boost::noncopyable {
    public:
      virtual ~Event() = default;

      //! Signals an event.
      using EventSignal = boost::signals2::signal<void ()>;

      //! Executes the event.
      virtual void Execute();

      //! Connects a slot to the EventSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectEventSignal(
        const EventSignal::slot_type& slot) const;

    protected:

      //! Constructs an Event.
      Event() = default;

      //! Signals that an event occurred.
      void SignalEvent();

    private:
      mutable EventSignal m_eventSignal;
  };

  inline void Event::Execute() {}

  inline boost::signals2::connection Event::ConnectEventSignal(
      const EventSignal::slot_type& slot) const {
    return m_eventSignal.connect(slot);
  }

  inline void Event::SignalEvent() {
    m_eventSignal();
  }
}
}

#endif
