#ifndef BEAM_FIXEDTIMECLIENT_HPP
#define BEAM_FIXEDTIMECLIENT_HPP
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/TimeService/TimeService.hpp"

namespace Beam {
namespace TimeService {

  /*! \class FixedTimeClient
      \brief A TimeClient whose value is set programmatically.
   */
  class FixedTimeClient : private boost::noncopyable {
    public:

      //! Constructs a FixedTimeClient.
      FixedTimeClient() = default;

      //! Constructs a FixedTimeClient.
      /*!
        \param time The time to use.
      */
      FixedTimeClient(const boost::posix_time::ptime& time);

      ~FixedTimeClient();

      //! Sets the time to use.
      void SetTime(const boost::posix_time::ptime& time);

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      IO::OpenState m_openState;
      boost::posix_time::ptime m_time;

      void Shutdown();
  };

  inline FixedTimeClient::FixedTimeClient(const boost::posix_time::ptime& time)
      : m_time{time} {}

  inline FixedTimeClient::~FixedTimeClient() {
    Close();
  }

  inline void FixedTimeClient::SetTime(const boost::posix_time::ptime& time) {
    m_time = time;
  }

  inline boost::posix_time::ptime FixedTimeClient::GetTime() {
    return m_time;
  }

  inline void FixedTimeClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void FixedTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void FixedTimeClient::Shutdown() {
    m_openState.SetClosed();
  }
}
}

#endif
