#ifndef BEAM_TIMER_HPP
#define BEAM_TIMER_HPP
#include <functional>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam {
namespace Threading {
namespace Details {
  BEAM_ENUM(TimerResult,

    //! The Timer expired.
    EXPIRED,

    //! The Timer was canceled.
    CANCELED,

    //! The Timer encountered an error.
    FAIL);
}

  /*! \class TimerConcept
      \brief Stores the semantics of the timer concept.
   */
  struct Timer : Concept<Timer> {

    /*! \enum Result
        \brief Enumerates the result of a Timer expiry.
     */
    using Result = Details::TimerResult;

    //! Starts the Timer.
    void Start();

    //! Cancels the Timer.
    void Cancel();

    //! Waits for the Timer to expire.
    void Wait();

    //! Returns the object publishing the result of a Start.
    const Publisher<Timer::Result>& GetPublisher() const;
  };
}
}

#endif
