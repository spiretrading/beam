#ifndef BEAM_NULLCONNECTION_HPP
#define BEAM_NULLCONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"

namespace Beam {
namespace IO {

  /*  \class NullConnection
      \brief A Connection used for testing purposes.
   */
  class NullConnection : private boost::noncopyable {
    public:

      //! Constructs a NullConnection.
      NullConnection();

      ~NullConnection();

      void Close();
  };

  inline NullConnection::NullConnection() {}

  inline NullConnection::~NullConnection() {
    Close();
  }

  inline void NullConnection::Close() {}
}

  template<>
  struct ImplementsConcept<IO::NullConnection, IO::Connection> :
    std::true_type {};
}

#endif
