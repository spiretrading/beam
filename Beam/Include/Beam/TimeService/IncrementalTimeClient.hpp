#ifndef BEAM_INCREMENTALTIMECLIENT_HPP
#define BEAM_INCREMENTALTIMECLIENT_HPP
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/TimeService/TimeService.hpp"

namespace Beam {
namespace TimeService {

  /*! \class IncrementalTimeClient
      \brief A TimeClient that increments its internal clock by a fixed amount
             every time the current time is requested.  Useful for testing and
             debugging.
   */
  class IncrementalTimeClient : private boost::noncopyable {
    public:

      //! Constructs an IncrementalTimeClient using the current universal time
      //! as the initial time and increments of 1 second.
      IncrementalTimeClient();

      //! Constructs an IncrementalTimeClient.
      /*!
        \param initialTime The initial time to use.
        \param increment The amount to increment the time by on each request.
      */
      IncrementalTimeClient(const boost::posix_time::ptime& initialTime,
        const boost::posix_time::time_duration& increment);

      ~IncrementalTimeClient();

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      IO::OpenState m_openState;
      boost::posix_time::ptime m_currentTime;
      boost::posix_time::time_duration m_increment;

      void Shutdown();
  };

  inline IncrementalTimeClient::IncrementalTimeClient()
      : m_currentTime(boost::posix_time::second_clock::universal_time()),
        m_increment(boost::posix_time::seconds(1)) {}

  inline IncrementalTimeClient::IncrementalTimeClient(
      const boost::posix_time::ptime& initialTime,
      const boost::posix_time::time_duration& increment)
      : m_currentTime(initialTime),
        m_increment(increment) {}

  inline IncrementalTimeClient::~IncrementalTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime IncrementalTimeClient::GetTime() {
    boost::posix_time::ptime time = m_currentTime;
    m_currentTime += m_increment;
    return time;
  }

  inline void IncrementalTimeClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  inline void IncrementalTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void IncrementalTimeClient::Shutdown() {
    m_openState.SetClosed();
  }
}
}

#endif
