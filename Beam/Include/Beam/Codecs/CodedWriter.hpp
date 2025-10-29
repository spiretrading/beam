#ifndef BEAM_CODED_WRITER_HPP
#define BEAM_CODED_WRITER_HPP
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Writes coded data using a Decoder.
   * @tparam W The type of Writer to write the encoded data to.
   * @tparam E The type of Encoder to use.
   */
  template<typename W, typename E> requires
    IsWriter<dereference_t<W>> && IsEncoder<dereference_t<E>>
  class CodedWriter {
    public:

      /** The type of Writer to write the encoded data to. */
      using DestinationWriter = dereference_t<W>;

      /** The type of Encoder to use. */
      using Encoder = dereference_t<E>;

      /**
       * Constructs a CodedWriter.
       * @param destination The destination to write to.
       * @param encoder The Encoder to use.
       */
      template<Initializes<W> WF, Initializes<E> EF>
      CodedWriter(WF&& destination, EF&& encoder);

      template<IsConstBuffer B>
      void write(const B& buffer);

    private:
      local_ptr_t<W> m_destination;
      local_ptr_t<E> m_encoder;

      CodedWriter(const CodedWriter&) = delete;
      CodedWriter& operator =(const CodedWriter&) = delete;
  };

  template<typename W, typename E>
  CodedWriter(W&&, E&&) ->
    CodedWriter<std::remove_cvref_t<W>, std::remove_cvref_t<E>>;

  template<typename W, typename E> requires
    IsWriter<dereference_t<W>> && IsEncoder<dereference_t<E>>
  template<Initializes<W> WF, Initializes<E> EF>
  CodedWriter<W, E>::CodedWriter(WF&& destination, EF&& encoder)
    : m_destination(std::forward<WF>(destination)),
      m_encoder(std::forward<EF>(encoder)) {}

  template<typename W, typename E> requires
    IsWriter<dereference_t<W>> && IsEncoder<dereference_t<E>>
  template<IsConstBuffer B>
  void CodedWriter<W, E>::write(const B& data) {
    auto output = SharedBuffer();
    try {
      m_encoder->encode(data, out(output));
    } catch(const std::exception&) {
      std::throw_with_nested(IOException("Encoder failed."));
    }
    m_destination->write(output);
  }
}

#endif
