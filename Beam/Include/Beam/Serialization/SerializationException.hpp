#ifndef BEAM_SERIALIZATIONEXCEPTION_HPP
#define BEAM_SERIALIZATIONEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Serialization/Serialization.hpp"

namespace Beam {
namespace Serialization {

  /*! \class SerializationException
      \brief Indicates that a serialization operation failed.
   */
  class SerializationException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a SerializationException.
      /*!
        \param what The error message.
      */
      SerializationException(const std::string& what);
  };

  inline SerializationException::SerializationException(const std::string& what)
    : std::runtime_error(what) {}
}
}

#endif
