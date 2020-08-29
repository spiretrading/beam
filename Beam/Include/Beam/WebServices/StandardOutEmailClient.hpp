#ifndef BEAM_STANDARDOUTEMAILCLIENT_HPP
#define BEAM_STANDARDOUTEMAILCLIENT_HPP
#include <iostream>
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/WebServices/Email.hpp"
#include "Beam/WebServices/EmailClient.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class StandardOutEmailClient.
      \brief Displays emails to stdout.
   */
  class StandardOutEmailClient : private boost::noncopyable {
    public:
      StandardOutEmailClient();

      ~StandardOutEmailClient();

      void Send(const Email& email);

      void Close();

    private:
      IO::OpenState m_openState;
  };

  inline StandardOutEmailClient::StandardOutEmailClient() {
    m_openState.SetOpen();
  }

  inline StandardOutEmailClient::~StandardOutEmailClient() {
    Close();
  }

  inline void StandardOutEmailClient::Send(const Email& email) {
    std::cout << email << std::flush;
  }

  inline void StandardOutEmailClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_openState.SetClosed();
  }
}
}

#endif
