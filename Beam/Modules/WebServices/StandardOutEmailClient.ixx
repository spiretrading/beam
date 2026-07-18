module;
#include "Prelude.hpp"

export module Beam:StandardOutEmailClient;

import :EmailClient;

export namespace Beam {

  /** Displays emails to stdout. */
  class StandardOutEmailClient {
    public:
      StandardOutEmailClient() = default;
      ~StandardOutEmailClient();

      void send(const Email& email);
      void close();

    private:
      OpenState m_open_state;
  };

  inline StandardOutEmailClient::~StandardOutEmailClient() {
    close();
  }

  inline void StandardOutEmailClient::send(const Email& email) {
    m_open_state.ensure_open();
    if(m_open_state.is_open()) {
      std::cout << email << std::flush;
    }
  }

  inline void StandardOutEmailClient::close() {
    m_open_state.close();
  }
}

