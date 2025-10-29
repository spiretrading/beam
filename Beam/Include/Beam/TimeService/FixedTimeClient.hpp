#ifndef BEAM_FIXED_TIME_CLIENT_HPP
#define BEAM_FIXED_TIME_CLIENT_HPP
#include <atomic>
#include "Beam/IO/OpenState.hpp"
#include "Beam/TimeService/TimeClient.hpp"

namespace Beam {

  /** A TimeClient whose value is set programmatically. */
  class FixedTimeClient {
    public:

      /** Constructs a FixedTimeClient. */
      FixedTimeClient() = default;

      /**
       * Constructs a FixedTimeClient.
       * @param time The time to use.
       */
      explicit FixedTimeClient(boost::posix_time::ptime time);

      ~FixedTimeClient();

      /** Sets the time to use. */
      void set(boost::posix_time::ptime time);

      boost::posix_time::ptime get_time();
      void close();

    private:
      OpenState m_open_state;
      std::atomic<boost::posix_time::ptime> m_time;

      FixedTimeClient(const FixedTimeClient&) = delete;
      FixedTimeClient& operator =(const FixedTimeClient&) = delete;
  };

  inline FixedTimeClient::FixedTimeClient(boost::posix_time::ptime time)
    : m_time(time) {}

  inline FixedTimeClient::~FixedTimeClient() {
    close();
  }

  inline void FixedTimeClient::set(boost::posix_time::ptime time) {
    m_open_state.ensure_open();
    m_time.store(time);
  }

  inline boost::posix_time::ptime FixedTimeClient::get_time() {
    m_open_state.ensure_open();
    return m_time.load();
  }

  inline void FixedTimeClient::close() {
    m_open_state.close();
  }
}

#endif
