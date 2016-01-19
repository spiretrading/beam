#ifndef BEAM_CODEDWRITER_HPP
#define BEAM_CODEDWRITER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Codecs {

  /*! \class CodedWriter
      \brief Writes coded data using a Decoder.
      \tparam DestinationWriterType The type of Writer to write the encoded data
              to.
      \tparam EncoderType The type of Encoder to use.
   */
  template<typename DestinationWriterType, typename EncoderType>
  class CodedWriter : private boost::noncopyable {
    public:

      //! The type of Writer to write the encoded data to.
      using DestinationWriter = GetTryDereferenceType<DestinationWriterType>;

      //! The type of Buffer used for encoding.
      using Buffer = typename DestinationWriter::Buffer;

      //! The type of Encoder to use.
      using Encoder = GetTryDereferenceType<EncoderType>;

      //! Constructs a CodedWriter.
      /*!
        \param destination The destination to write to.
        \param encoder The Encoder to use.
      */
      template<typename DestinationWriterForward, typename EncoderForward>
      CodedWriter(DestinationWriterForward&& destination,
        EncoderForward&& encoder);

      void Write(const void* data, std::size_t size);

      template<typename BufferType>
      void Write(const BufferType& data);

    private:
      GetOptionalLocalPtr<DestinationWriterType> m_destination;
      GetOptionalLocalPtr<EncoderType> m_encoder;
  };

  template<typename DestinationWriter, typename EncoderType>
  template<typename DestinationWriterForward, typename EncoderForward>
  CodedWriter<DestinationWriter, EncoderType>::CodedWriter(
      DestinationWriterForward&& destination, EncoderForward&& encoder)
      : m_destination(std::forward<DestinationWriterForward>(destination)),
        m_encoder(std::forward<EncoderForward>(encoder)) {}

  template<typename DestinationWriter, typename EncoderType>
  void CodedWriter<DestinationWriter, EncoderType>::Write(const void* data,
      std::size_t size) {
    Buffer encodedData;
    m_encoder->Encode(data, size, Store(encodedData));
    m_destination->Write(encodedData);
  }

  template<typename DestinationWriter, typename EncoderType>
  template<typename BufferType>
  void CodedWriter<DestinationWriter, EncoderType>::Write(
      const BufferType& data) {
    Write(data.GetData(), data.GetSize());
  }
}

  template<typename DestinationWriter, typename EncoderType>
  struct ImplementsConcept<Codecs::CodedWriter<DestinationWriter, EncoderType>,
      IO::Writer<typename Codecs::CodedWriter<
      DestinationWriter, EncoderType>::Buffer>> : std::true_type {};
}

#endif
