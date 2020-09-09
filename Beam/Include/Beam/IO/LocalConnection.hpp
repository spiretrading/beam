#ifndef BEAM_LOCAL_CONNECTION_HPP
#define BEAM_LOCAL_CONNECTION_HPP
#include <atomic>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/PipedWriter.hpp"

namespace Beam {
namespace IO {

  /**
   * Implements a local Connection.
   * @param <B> The type of Buffer to use.
   */
  template<typename B>
  class LocalConnection {
    public:

      /** The type of Buffer to use. */
      using Buffer = B;

      /** The type of LocalServerConnection this connects to. */
      using LocalServerConnection = IO::LocalServerConnection<Buffer>;

      /**
       * Constructs a LocalConnection.
       * @param writer The Writer to use.
       * @param endpointWriter The Writer being used by the endpoint.
       */
      LocalConnection(std::shared_ptr<PipedWriter<Buffer>> writer,
        std::shared_ptr<PipedWriter<Buffer>> endpointWriter);

      ~LocalConnection();

      void Close();

    private:
      std::shared_ptr<PipedWriter<Buffer>> m_writer;
      std::shared_ptr<PipedWriter<Buffer>> m_endpointWriter;
      OpenState m_openState;

      LocalConnection(const LocalConnection&) = delete;
      LocalConnection& operator =(const LocalConnection&) = delete;
  };

  template<typename B>
  LocalConnection<B>::LocalConnection(
    std::shared_ptr<PipedWriter<Buffer>> writer,
    std::shared_ptr<PipedWriter<Buffer>> endpointWriter)
    : m_writer(std::move(writer)),
      m_endpointWriter(std::move(endpointWriter)) {}

  template<typename B>
  LocalConnection<B>::~LocalConnection() {
    Close();
  }

  template<typename B>
  void LocalConnection<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_endpointWriter->Break();
    m_writer->Break();
    m_openState.Close();
  }
}

  template<typename B>
  struct ImplementsConcept<IO::LocalConnection<B>, IO::Connection> :
    std::true_type {};
}

#endif
