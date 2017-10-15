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
      enum class Update {

        //! No update to the Reactor.
        NONE,

        //! The Reactor has come to an end with no update.
        COMPLETE,

        //! The Reactor's evaluation has updated
        //! (it may also have come to an end).
        EVAL
      };

      virtual ~BaseReactor() = default;

      //! Returns <code>true</code> iff this Reactor is initialized.
      virtual bool IsInitialized() const = 0;

      //! Returns <code>true</code> iff this Reactor is complete.
      virtual bool IsComplete() const = 0;

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
