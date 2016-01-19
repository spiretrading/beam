#ifndef BEAM_NOTSUPPORTEDEXCEPTION_HPP
#define BEAM_NOTSUPPORTEDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class NotSupportedException
      \brief Indicates that an optional virtual method is not supported.
   */
  class NotSupportedException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a NotSupportedException.
      /*!
        \param message Typically the name of the method that's not supported.
      */
      NotSupportedException(const std::string& message);
  };

  inline NotSupportedException::NotSupportedException(
      const std::string& message)
      : std::runtime_error(message) {}
}

#endif
