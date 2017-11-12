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

  inline BaseReactor::Update Combine(BaseReactor::Update& lhs,
      BaseReactor::Update rhs) {
    lhs = static_cast<BaseReactor::Update>(
      static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
  }

  //! Returns <code>true</code> iff a Reactor Update represents its completion.
  inline bool IsComplete(BaseReactor::Update update) {
    return (static_cast<int>(update) &
      static_cast<int>(BaseReactor::Update::COMPLETE)) != 0;
  }

  //! Returns <code>true</code> iff a Reactor Update has an evaluation.
  inline bool HasEval(BaseReactor::Update update) {
    return (static_cast<int>(update) &
      static_cast<int>(BaseReactor::Update::EVAL)) != 0;
  }
}
}

#endif
