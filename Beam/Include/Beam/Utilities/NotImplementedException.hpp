#ifndef BEAM_NOTIMPLEMENTEDEXCEPTION_HPP
#define BEAM_NOTIMPLEMENTEDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class NotImplementedException
      \brief Indicates that a function/method has not been implemented.
   */
  class NotImplementedException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a NotImplementedException.
      NotImplementedException();

      //! Constructs a NotImplementedException.
      /*!
        \param message A message describing the error.
      */
      NotImplementedException(const std::string& message);
  };

  inline NotImplementedException::NotImplementedException()
      : std::runtime_error("Function is not implemented.") {}

  inline NotImplementedException::NotImplementedException(
      const std::string& message)
      : std::runtime_error(message) {}
}

#endif
