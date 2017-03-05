#ifndef BEAM_STOMPEXCEPTION_HPP
#define BEAM_STOMPEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Stomp/Stomp.hpp"

namespace Beam {
namespace Stomp {

  /*! \class StompException
      \brief Signals a protocol related exception.
   */
  class StompException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a StompException.
      /*!
        \param message A message describing the error.
      */
      StompException(const std::string& message);
  };

  inline StompException::StompException(const std::string& message)
    : std::runtime_error{message} {}
}
}

#endif
