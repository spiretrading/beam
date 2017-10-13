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
      virtual ~BaseReactor() = default;

      //! Returns <code>true</code> iff this Reactor is complete.
      virtual bool IsComplete() const = 0;

      //! Returns the evaluation's type_info.
      virtual const std::type_info& GetType() const = 0;

      //! Commits changes to this Reactor.
      /*!
        \param sequenceNumber The sequence number representing the change.
      */
      virtual void Commit(int sequenceNumber) = 0;

    protected:

      //! Constructs a BaseReactor.
      BaseReactor() = default;
  };
}
}

#endif
