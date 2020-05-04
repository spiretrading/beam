#ifndef BEAM_BASICOSTREAMWRITER_HPP
#define BEAM_BASICOSTREAMWRITER_HPP
#include <ostream>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class BasicOStreamWriter
      \brief Wraps an std::basic_ostream for use with the Writer interface.
      \tparam OStreamType The type of ostream to write to.
   */
  template<typename OStreamType>
  class BasicOStreamWriter : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      //! The type of OStream being written to.
      using OStream = typename TryDereferenceType<OStreamType>::type;

      //! Constructs a BasicOStreamWriter.
      /*!
        \param destination The basic_ostream to wrap.
      */
      template<typename OStreamForward>
      BasicOStreamWriter(OStreamForward&& destination);

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      typename OptionalLocalPtr<OStreamType>::type m_destination;
  };

  template<typename OStreamType>
  template<typename OStreamForward>
  BasicOStreamWriter<OStreamType>::BasicOStreamWriter(
      OStreamForward&& destination)
      : m_destination(std::forward<OStreamForward>(destination)) {}

  template<typename OStreamType>
  void BasicOStreamWriter<OStreamType>::Write(const void* data,
      std::size_t size) {
    m_destination->write(static_cast<const typename OStream::char_type*>(data),
      size / sizeof(typename OStream::char_type));
  }

  template<typename OStreamType>
  template<typename BufferType>
  void BasicOStreamWriter<OStreamType>::Write(const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }
}

  template<typename BufferType, typename OStreamType>
  struct ImplementsConcept<IO::BasicOStreamWriter<OStreamType>,
    IO::Writer<BufferType>> : std::true_type {};
}

#endif
