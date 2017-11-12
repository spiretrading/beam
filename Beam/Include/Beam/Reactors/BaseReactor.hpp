#ifndef BEAM_BASE_REACTOR_HPP
#define BEAM_BASE_REACTOR_HPP
#include <typeinfo>
#include <boost/noncopyable.hpp>
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class BaseReactor
      \brief The untyped base class of a Reactor.
   */
  class BaseReactor : private boost::noncopyable {
    public:

      /*! \enum Update
          \brief Lists the types of updates a Reactor can undergo after a
                 commit.
       */
      enum class Update : int {

        //! No update to the Reactor.
        NONE = 0,

        //! The Reactor has come to an end with no update.
        COMPLETE = 1,

        //! The Reactor's evaluation has updated
        //! (it may also have come to an end).
        EVAL = 2,

        //! The Reactor is both complete and has an evaluation.
        COMPLETE_WITH_EVAL = COMPLETE | EVAL
      };

      virtual ~BaseReactor() = default;

      //! Returns the evaluation's type_info.
      virtual const std::type_info& GetType() const = 0;

      //! Commits changes to this Reactor.
      /*!
        \param sequenceNumber The sequence number representing the change.
        \return A code representing the effect the commit had on this Reactor.
      */
      virtual Update Commit(int sequenceNumber) = 0;

    protected:

      //! Constructs a BaseReactor.
      BaseReactor() = default;
  };
}
}

#endif
