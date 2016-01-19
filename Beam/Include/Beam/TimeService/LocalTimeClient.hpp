#ifndef BEAM_LOCALTIMECLIENT_HPP
#define BEAM_LOCALTIMECLIENT_HPP
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/TimeService/TimeService.hpp"

namespace Beam {
namespace TimeService {

  /*! \class LocalTimeClient
      \brief A TimeClient that uses the local computer's clock.
   */
  class LocalTimeClient : private boost::noncopyable {
    public:

      //! Constructs a LocalTimeClient.
      LocalTimeClient();

      ~LocalTimeClient();

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      IO::OpenState m_openState;

      void Shutdown();
  };

  inline LocalTimeClient::LocalTimeClient() {}

  inline LocalTimeClient::~LocalTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime LocalTimeClient::GetTime() {
    return boost::posix_time::microsec_clock::universal_time();
  }

  inline void LocalTimeClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void LocalTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void LocalTimeClient::Shutdown() {
    m_openState.SetClosed();
  }
}
}

#endif
