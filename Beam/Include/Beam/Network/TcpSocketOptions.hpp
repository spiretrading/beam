#ifndef BEAM_NETWORK_TCP_SOCKET_OPTIONS_HPP
#define BEAM_NETWORK_TCP_SOCKET_OPTIONS_HPP

namespace Beam::Network {

  /** Stores the various options that can be applied to a TcpSocketChannel. */
  struct TcpSocketOptions {

    /** <code>true</code> iff the TCP no delay option should be enabled. */
    bool m_noDelayEnabled;

    /** The size of the write buffer. */
    int m_writeBufferSize;

    /** Constructs the default options. */
    TcpSocketOptions();
  };

  inline TcpSocketOptions::TcpSocketOptions()
    : m_noDelayEnabled(false),
      m_writeBufferSize(8 * 1024) {}
}

#endif
