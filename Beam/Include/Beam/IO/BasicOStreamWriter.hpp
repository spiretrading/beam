#ifndef BEAM_BASIC_OSTREAM_WRITER_HPP
#define BEAM_BASIC_OSTREAM_WRITER_HPP
#include <ostream>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /**
   * Wraps an std::basic_ostream for use with the Writer interface.
   * @param <S> The type of ostream to write to.
   */
  template<typename S>
  class BasicOStreamWriter {
    public:
      using Buffer = SharedBuffer;

      /** The type of OStream being written to. */
      using OStream = GetTryDereferenceType<S>;

      /**
       * Constructs a BasicOStreamWriter.
       * @param destination The basic_ostream to wrap.
       */
      template<typename SF>
      BasicOStreamWriter(SF&& destination);

      void Write(const void* data, std::size_t size);

      template<typename B>
      void Write(const B& data);

    private:
      GetOptionalLocalPtr<S> m_destination;

      BasicOStreamWriter(const BasicOStreamWriter&) = delete;
      BasicOStreamWriter& operator =(const BasicOStreamWriter&) = delete;
  };

  template<typename S>
  template<typename SF>
  BasicOStreamWriter<S>::BasicOStreamWriter(SF&& destination)
    : m_destination(std::forward<SF>(destination)) {}

  template<typename S>
  void BasicOStreamWriter<S>::Write(const void* data, std::size_t size) {
    m_destination->write(static_cast<const typename OStream::char_type*>(data),
      size / sizeof(typename OStream::char_type));
  }

  template<typename S>
  template<typename B>
  void BasicOStreamWriter<S>::Write(const B& data) {
    Write(data.GetData(), data.GetSize());
  }
}

  template<typename BufferType, typename S>
  struct ImplementsConcept<IO::BasicOStreamWriter<S>, IO::Writer<BufferType>> :
    std::true_type {};
}

#endif
