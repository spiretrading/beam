#ifndef BEAM_IO_HPP
#define BEAM_IO_HPP

namespace Beam::IO {
  template<typename DestinationWriterType> class AsyncWriter;
  struct Buffer;
  template<typename BufferType> class BufferView;
  template<typename IStreamType> class BasicIStreamReader;
  template<typename OStreamType> class BasicOStreamWriter;
  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType> struct Channel;
  struct ChannelIdentifier;
  template<typename ServerConnectionType, typename ChannelType>
    class ChannelAdapterServerConnection;
  class EndOfFileException;
  class IOException;
  template<typename BufferType> class LocalClientChannel;
  template<typename BufferType> class LocalConnection;
  template<typename BufferType> class LocalServerChannel;
  template<typename BufferType> class LocalServerChannelConnection;
  template<typename BufferType> class LocalServerConnection;
  class NamedChannelIdentifier;
  class NullChannel;
  class NullConnection;
  class NullReader;
  class NullWriter;
  class OpenState;
  template<typename BufferType> class PipedReader;
  template<typename BufferType> class PipedWriter;
  template<typename BufferType, typename SourceReaderType> class QueuedReader;
  template<typename BufferType> struct Reader;
  class SharedBuffer;
  template<typename SourceReaderType> class SizeDeclarativeReader;
  template<typename DestinationWriterType> class SizeDeclarativeWriter;
  template<std::size_t> class StaticBuffer;
  class VirtualChannel;
  class VirtualChannelIdentifier;
  class VirtualConnection;
  class VirtualReader;
  class VirtualServerConnection;
  class VirtualWriter;
  template<typename C> class WrapperVirtualChannel;
  template<typename I> class WrapperChannelIdentifier;
  template<typename C> class WrapperConnection;
  template<typename R> class WrapperReader;
  template<typename C> class WrapperServerConnection;
  template<typename W> class WrapperWriter;
  template<typename BufferType> struct Writer;
}

#endif
