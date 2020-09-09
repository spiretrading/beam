#ifndef BEAM_FIXED_TIME_CLIENT_HPP
#define BEAM_FIXED_TIME_CLIENT_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/TimeService/TimeService.hpp"

namespace Beam::TimeService {

  /** A TimeClient whose value is set programmatically. */
  class FixedTimeClient {
    public:

      /** Constructs a FixedTimeClient. */
      FixedTimeClient() = default;

      /**
       * Constructs a FixedTimeClient.
       * @param time The time to use.
       */
      FixedTimeClient(const boost::posix_time::ptime& time);

      ~FixedTimeClient();

      /** Sets the time to use. */
      void SetTime(const boost::posix_time::ptime& time);

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      IO::OpenState m_openState;
      boost::posix_time::ptime m_time;

      FixedTimeClient(const FixedTimeClient&) = delete;
      FixedTimeClient& operator =(const FixedTimeClient&) = delete;
  };

  inline FixedTimeClient::FixedTimeClient(const boost::posix_time::ptime& time)
    : m_time(time) {}

  inline FixedTimeClient::~FixedTimeClient() {
    Close();
  }

  inline void FixedTimeClient::SetTime(const boost::posix_time::ptime& time) {
    m_time = time;
  }

  inline boost::posix_time::ptime FixedTimeClient::GetTime() {
    return m_time;
  }

  inline void FixedTimeClient::Close() {
    m_openState.Close();
  }
}

#endif
