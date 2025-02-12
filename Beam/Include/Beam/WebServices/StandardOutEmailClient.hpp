#ifndef BEAM_STANDARD_OUT_EMAIL_CLIENT_HPP
#define BEAM_STANDARD_OUT_EMAIL_CLIENT_HPP
#include <iostream>
#include "Beam/IO/OpenState.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Displays emails to stdout. */
  class StandardOutEmailClient {
    public:
      StandardOutEmailClient() = default;

      ~StandardOutEmailClient();

      void Send(const Email& email);

      void Close();

    private:
      IO::OpenState m_openState;
  };

  inline StandardOutEmailClient::~StandardOutEmailClient() {
    Close();
  }

  inline void StandardOutEmailClient::Send(const Email& email) {
    if(m_openState.IsOpen()) {
      std::cout << email << std::flush;
    }
  }

  inline void StandardOutEmailClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.Close();
  }
}

#endif
