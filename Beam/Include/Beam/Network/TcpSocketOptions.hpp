#ifndef BEAM_NETWORK_TCP_SOCKET_OPTIONS_HPP
#define BEAM_NETWORK_TCP_SOCKET_OPTIONS_HPP

namespace Beam {

  /** Stores the various options that can be applied to a TcpSocketChannel. */
  struct TcpSocketOptions {

    /** <code>true</code> iff the TCP no delay option should be enabled. */
    bool m_no_delay_enabled;

    /** The size of the write buffer. */
    int m_write_buffer_size;

    /** Constructs the default options. */
    TcpSocketOptions() noexcept;
  };

  inline TcpSocketOptions::TcpSocketOptions() noexcept
    : m_no_delay_enabled(false),
      m_write_buffer_size(8 * 1024) {}
}

#endif
