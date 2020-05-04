#ifndef BEAM_SIZEDECLARATIVEWRITER_HPP
#define BEAM_SIZEDECLARATIVEWRITER_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class SizeDeclarativeWriter
      \brief Writes to a destination, declaring the size to be written.
      \tparam DestinationWriterType The Writer to write to.
   */
  template<typename DestinationWriterType>
  class SizeDeclarativeWriter : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      //! The destination to write to.
      using DestinationWriter =
        typename TryDereferenceType<DestinationWriterType>::type;

      //! Constructs a SizeDeclarativeWriter.
      /*!
        \param destination Used to initialize the destination of all writes.
      */
      template<typename DestinationWriterForward>
      SizeDeclarativeWriter(DestinationWriterForward&& destination);

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      typename OptionalLocalPtr<DestinationWriterType>::type m_destination;
  };

  template<typename DestinationWriterType>
  template<typename DestinationWriterForward>
  SizeDeclarativeWriter<DestinationWriterType>::SizeDeclarativeWriter(
      DestinationWriterForward&& destination)
      : m_destination(std::forward<DestinationWriterForward>(destination)) {}

  template<typename DestinationWriterType>
  void SizeDeclarativeWriter<DestinationWriterType>::Write(const void* data,
      std::size_t size) {
    std::uint32_t portableInt = ToLittleEndian(
      static_cast<std::uint32_t>(size));
    typename DestinationWriter::Buffer buffer;
    buffer.Append(reinterpret_cast<const char*>(&portableInt),
      sizeof(std::uint32_t));
    buffer.Append(data, size);
    m_destination->Write(buffer);
  }

  template<typename DestinationWriterType>
  template<typename BufferType>
  void SizeDeclarativeWriter<DestinationWriterType>::Write(
      const BufferType& data) {
    std::uint32_t portableInt = ToLittleEndian(
      static_cast<std::uint32_t>(data.GetSize()));
    typename DestinationWriter::Buffer buffer;
    buffer.Append(reinterpret_cast<const char*>(&portableInt),
      sizeof(std::uint32_t));
    buffer.Append(data);
    m_destination->Write(std::move(buffer));
  }
}

  template<typename BufferType, typename DestinationWriterType>
  struct ImplementsConcept<IO::SizeDeclarativeWriter<DestinationWriterType>,
    IO::Writer<BufferType>> : std::true_type {};
}

#endif
