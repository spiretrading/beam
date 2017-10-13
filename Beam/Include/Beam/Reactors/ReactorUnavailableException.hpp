#ifndef BEAM_REACTOR_UNAVAILABLE_EXCEPTION_HPP
#define BEAM_REACTOR_UNAVAILABLE_EXCEPTION_HPP
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorUnavailableException
      \brief Signals that a Reactor is unavailable.
   */
  class ReactorUnavailableException : public ReactorException {
    public:

      //! Constructs a ReactorUnavailableException.
      ReactorUnavailableException();

      //! Constructs a ReactorUnavailableException.
      /*!
        \param message A message describing the exception.
      */
      ReactorUnavailableException(const std::string& message);

      virtual ~ReactorUnavailableException() throw();
  };

  inline ReactorUnavailableException::ReactorUnavailableException() {}

  inline ReactorUnavailableException::ReactorUnavailableException(
      const std::string& message)
      : ReactorException(message) {}

  inline ReactorUnavailableException::~ReactorUnavailableException()
    throw() {}
}
}

#endif
