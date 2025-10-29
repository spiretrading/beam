#ifndef BEAM_NETWORK_UDP_SOCKET_OPTIONS_HPP
#define BEAM_NETWORK_UDP_SOCKET_OPTIONS_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Beam {

  /** Stores the various options that can be applied to a UdpSocket. */
  struct UdpSocketOptions {

    /** The timeout on receive operations. */
    boost::posix_time::time_duration m_timeout;

    /** The TTL setting. */
    int m_ttl;

    /** Whether to enable loopback. */
    bool m_enable_loopback;

    /** The max datagram size. */
    std::size_t m_max_datagram_size;

    /** The default size of the receive buffer. */
    std::size_t m_receive_buffer_size;

    /** Constructs the default options. */
    UdpSocketOptions() noexcept;
  };

  inline UdpSocketOptions::UdpSocketOptions() noexcept
    : m_timeout(boost::posix_time::pos_infin),
      m_ttl(-1),
      m_max_datagram_size(2 * 1024),
      m_receive_buffer_size(8 * 1024) {}
}

#endif
