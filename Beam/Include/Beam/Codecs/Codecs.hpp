#ifndef BEAM_CODECS_HPP
#define BEAM_CODECS_HPP

namespace Beam::Codecs {
  template<typename BufferType, typename SourceReaderType, typename DecoderType>
    class CodedReader;
  template<typename DestinationWriterType, typename EncoderType>
    class CodedWriter;
  template<typename DestinationWriter, typename EncoderType> class CodedWriter;
  struct Decoder;
  class DecoderException;
  struct Encoder;
  class EncoderException;
  class NullDecoder;
  class NullEncoder;
  template<typename DecoderType> class SizeDeclarativeDecoder;
  template<typename EncoderType> class SizeDeclarativeEncoder;
  class ZLibDecoder;
  class ZLibEncoder;
}

#endif
