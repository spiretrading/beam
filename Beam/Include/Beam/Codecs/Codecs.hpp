#ifndef BEAM_CODECS_HPP
#define BEAM_CODECS_HPP

namespace Beam::Codecs {
  template<typename B, typename R, typename D> class CodedReader;
  template<typename W, typename E> class CodedWriter;
  struct Decoder;
  class DecoderException;
  struct Encoder;
  class EncoderException;
  class NullDecoder;
  class NullEncoder;
  template<typename D> class SizeDeclarativeDecoder;
  template<typename E> class SizeDeclarativeEncoder;
  class ZLibDecoder;
  class ZLibEncoder;
}

#endif
