#ifndef BEAM_SIZE_DECLARATIVE_WRITER_HPP
#define BEAM_SIZE_DECLARATIVE_WRITER_HPP
#include <type_traits>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /**
   * Writes to a destination, declaring the size to be written.
   * @param <W> The Writer to write to.
   */
  template<typename W>
  class SizeDeclarativeWriter {
    public:
      using Buffer = SharedBuffer;

      /** The destination to write to. */
      using DestinationWriter = GetTryDereferenceType<W>;

      /**
       * Constructs a SizeDeclarativeWriter.
       * @param destination Used to initialize the destination of all writes.
       */
      template<typename WF>
      SizeDeclarativeWriter(WF&& destination);

      void Write(const void* data, std::size_t size);

      template<typename B>
      void Write(const B& data);

    private:
      GetOptionalLocalPtr<W> m_destination;

      SizeDeclarativeWriter(const SizeDeclarativeWriter&) = delete;
      SizeDeclarativeWriter& operator =(const SizeDeclarativeWriter&) = delete;
  };

  template<typename W>
  SizeDeclarativeWriter(W&&) -> SizeDeclarativeWriter<std::decay_t<W>>;

  template<typename W>
  template<typename WF>
  SizeDeclarativeWriter<W>::SizeDeclarativeWriter(WF&& destination)
    : m_destination(std::forward<WF>(destination)) {}

  template<typename W>
  void SizeDeclarativeWriter<W>::Write(const void* data, std::size_t size) {
    auto portableInt = ToLittleEndian(static_cast<std::uint32_t>(size));
    auto buffer = typename DestinationWriter::Buffer();
    try {
      buffer.Append(reinterpret_cast<const char*>(&portableInt),
        sizeof(std::uint32_t));
      buffer.Append(data, size);
    } catch(const std::exception&) {
      std::throw_with_nested(IOException());
    }
    m_destination->Write(std::move(buffer));
  }

  template<typename W>
  template<typename B>
  void SizeDeclarativeWriter<W>::Write(const B& data) {
    return Write(data.GetData(), data.GetSize());
  }
}

  template<typename B, typename W>
  struct ImplementsConcept<IO::SizeDeclarativeWriter<W>, IO::Writer<B>> :
    std::true_type {};
}

#endif
