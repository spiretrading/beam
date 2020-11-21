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
  struct ChannelIdentifier;
  template<typename S, typename C> class ChannelAdapterServerConnection;
  class ConnectException;
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
  template<typename B> struct Reader;
  template<typename C> struct ServerConnection;
  class SharedBuffer;
  template<typename R> class SizeDeclarativeReader;
  template<typename W> class SizeDeclarativeWriter;
  template<std::size_t N> class StaticBuffer;
  class VirtualChannel;
  class VirtualChannelIdentifier;
  class VirtualConnection;
  class VirtualReader;
  class VirtualServerConnection;
  class VirtualWriter;
  template<typename C> class WrapperVirtualChannel;
  template<typename C, typename CT1, typename CT2, typename CT3>
    class WrapperChannel;
  template<typename I> class WrapperChannelIdentifier;
  template<typename C> class WrapperConnection;
  template<typename R> class WrapperReader;
  template<typename C> class WrapperServerConnection;
  template<typename W> class WrapperWriter;
  template<typename B> struct Writer;
}

#endif
