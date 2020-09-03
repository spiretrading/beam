#ifndef BEAM_NETWORK_UDP_SOCKET_OPTIONS_HPP
#define BEAM_NETWORK_UDP_SOCKET_OPTIONS_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Beam::Network {

  /** Stores the various options that can be applied to a UdpSocket. */
  struct UdpSocketOptions {

    /** The timeout on receive operations. */
    boost::posix_time::time_duration m_timeout;

    /** The TTL setting. */
    int m_ttl;

    /** Whether to enable loopback. */
    bool m_enableLoopback;

    /** The max datagram size. */
    std::size_t m_maxDatagramSize;

    /** The default size of the receive buffer. */
    std::size_t m_receiveBufferSize;

    /** Constructs the default options. */
    UdpSocketOptions();
  };

  inline UdpSocketOptions::UdpSocketOptions()
    : m_timeout(boost::posix_time::pos_infin),
      m_ttl(-1),
      m_maxDatagramSize(2 * 1024),
      m_receiveBufferSize(8 * 1024) {}
}

#endif
