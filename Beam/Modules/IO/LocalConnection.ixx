module;
#include "Prelude.hpp"

export module Beam:LocalConnection;

export namespace Beam {

  /** Implements a local Connection. */
  class LocalConnection {
    public:

      /**
       * Constructs a LocalConnection.
       * @param writer The Writer to use.
       * @param end The Writer being used by the end.
       */
      LocalConnection(std::shared_ptr<PipedWriter> writer,
        std::shared_ptr<PipedWriter> end);

      ~LocalConnection();

      void close();

    private:
      std::shared_ptr<PipedWriter> m_writer;
      std::shared_ptr<PipedWriter> m_end;
      OpenState m_open_state;

      LocalConnection(const LocalConnection&) = delete;
      LocalConnection& operator =(const LocalConnection&) = delete;
  };

  inline LocalConnection::LocalConnection(
    std::shared_ptr<PipedWriter> writer, std::shared_ptr<PipedWriter> end)
    : m_writer(std::move(writer)),
      m_end(std::move(end)) {}

  inline LocalConnection::~LocalConnection() {
    close();
  }

  inline void LocalConnection::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_end->close();
    m_writer->close();
    m_open_state.close();
  }
}

