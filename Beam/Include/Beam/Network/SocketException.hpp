#ifndef BEAM_SOCKETEXCEPTION_HPP
#define BEAM_SOCKETEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/SocketIdentifier.hpp"

namespace Beam {
namespace Network {

  /*! \class SocketException
      \brief Indicates that an operation on a socket failed.
   */
  class SocketException : public IO::IOException {
    public:

      //! Constructs a SocketException.
      /*!
        \param code The error code, typically provided by the underlying system.
        \param message The error message.
      */
      SocketException(int code, const std::string& message);

      //! Returns the error code.
      int GetCode() const;

    private:
      int m_code;
  };

  inline SocketException::SocketException(int code, const std::string& message)
    : IOException(message),
      m_code(code) {}

  inline int SocketException::GetCode() const {
    return m_code;
  }
}
}

#endif
