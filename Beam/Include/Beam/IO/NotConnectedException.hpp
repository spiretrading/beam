#ifndef BEAM_NOTCONNECTEDEXCEPTION_HPP
#define BEAM_NOTCONNECTEDEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {
namespace IO {

  /*! \class NotConnectedException
      \brief Indicates an operation failed due to not being connected.
   */
  class NotConnectedException : public IOException {
    public:

      //! Constructs a NotConnectedException.
      NotConnectedException();

      //! Constructs a NotConnectedException.
      /*!
        \param message A message describing the error.
      */
      NotConnectedException(const std::string& message);
  };

  inline NotConnectedException::NotConnectedException()
    : IOException("Not connected.") {}

  inline NotConnectedException::NotConnectedException(
      const std::string& message)
      : IOException(message) {}
}
}

#endif
