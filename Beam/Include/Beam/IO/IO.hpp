#ifndef BEAM_IO_HPP
#define BEAM_IO_HPP
#include <cstddef>

namespace Beam::IO {
  template<typename W> class AsyncWriter;
  template<typename I, typename C, typename R, typename W> class BasicChannel;
  template<typename S> class BasicIStreamReader;
  template<typename S> class BasicOStreamWriter;
  struct Buffer;
  class BufferBox;
  template<typename B> class BaseBufferOutputStream;
  template<typename B> class BufferSlice;
  class BufferView;
  template<typename I, typename C, typename R, typename W> struct Channel;
  class ChannelBox;
  struct ChannelIdentifier;
  class ChannelIdentifierBox;
  template<typename S, typename C> class ChannelAdapterServerConnection;
  class ConnectException;
  struct Connection;
  class ConnectionBox;
  class EndOfFileException;
  class IOException;
  template<typename B> class LocalClientChannel;
  template<typename B> class LocalConnection;
  template<typename B> class LocalServerChannel;
  template<typename B> class LocalServerChannelConnection;
  template<typename B> class LocalServerConnection;
  class NamedChannelIdentifier;
  class NotConnectedException;
  class NullChannel;
  class NullConnection;
  class NullReader;
  class NullWriter;
  class OpenState;
  template<typename B> class PipedReader;
  template<typename B> class PipedWriter;
  template<typename B, typename R> class QueuedReader;
  struct Reader;
  class ReaderBox;
  template<typename C> struct ServerConnection;
  class ServerConnectionBox;
  class SharedBuffer;
  template<typename R> class SizeDeclarativeReader;
  template<typename W> class SizeDeclarativeWriter;
  template<std::size_t N> class StaticBuffer;
  template<typename B> struct Writer;
  class WriterBox;
}

#endif
