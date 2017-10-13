#ifndef BEAM_REACTOR_EXCEPTION_HPP
#define BEAM_REACTOR_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorException
      \brief Signals an exception in the evaluation of a Reactor.
   */
  class ReactorException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a ReactorException.
      ReactorException();

      //! Constructs a ReactorException.
      /*!
        \param message A message describing the exception.
      */
      ReactorException(const std::string& message);

      virtual ~ReactorException() throw();
  };

  inline ReactorException::ReactorException()
      : std::runtime_error{"Reactor failed."} {}

  inline ReactorException::ReactorException(const std::string& message)
      : std::runtime_error{message} {}

  inline ReactorException::~ReactorException() throw() {}
}
}

#endif
