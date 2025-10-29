#ifndef BEAM_CODED_READER_HPP
#define BEAM_CODED_READER_HPP
#include <limits>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /**
   * Reads coded data using a Decoder.
   * @tparam R The type of Reader to read encoded data from.
   * @tparam D The type of Decoder used.
   */
  template<typename R, typename D> requires
    IsReader<dereference_t<R>> && IsDecoder<dereference_t<D>>
  class CodedReader {
    public:

      /** The type of Reader to read from. */
      using SourceReader = dereference_t<R>;

      /** The type of Decoder used. */
      using Decoder = dereference_t<D>;

      /**
       * Constructs a CodedReader.
       * @param source Initializes the source to read from.
       * @param decoder Initializes the Decoder to use.
       */
      template<Initializes<R> SF, Initializes<D> DF>
      CodedReader(SF&& source, DF&& decoder);

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      local_ptr_t<R> m_source;
      local_ptr_t<D> m_decoder;
      PipedReader m_reader;
      PipedWriter m_writer;
      SharedBuffer m_source_buffer;
      SharedBuffer m_decoder_buffer;

      CodedReader(const CodedReader&) = delete;
      CodedReader& operator =(const CodedReader&) = delete;
      void read();
  };

  template<typename S, typename D>
  CodedReader(S&&, D&&) ->
    CodedReader<std::remove_cvref_t<S>, std::remove_cvref_t<D>>;

  template<typename R, typename D> requires
    IsReader<dereference_t<R>> && IsDecoder<dereference_t<D>>
  template<Initializes<R> SF, Initializes<D> DF>
  CodedReader<R, D>::CodedReader(SF&& source, DF&& decoder)
    : m_source(std::forward<SF>(source)),
      m_decoder(std::forward<DF>(decoder)),
      m_writer(Ref(m_reader)) {}

  template<typename R, typename D> requires
    IsReader<dereference_t<R>> && IsDecoder<dereference_t<D>>
  bool CodedReader<R, D>::poll() const {
    return m_reader.poll() || m_source->poll();
  }

  template<typename R, typename D> requires
    IsReader<dereference_t<R>> && IsDecoder<dereference_t<D>>
  template<IsBuffer B>
  std::size_t CodedReader<R, D>::read(Out<B> destination, std::size_t size) {
    read();
    return m_reader.read(out(destination), size);
  }

  template<typename R, typename D> requires
    IsReader<dereference_t<R>> && IsDecoder<dereference_t<D>>
  void CodedReader<R, D>::read() {
    if(m_reader.poll()) {
      return;
    }
    try {
      m_source->read(out(m_source_buffer));
    } catch(const std::exception&) {
      m_writer.close(std::current_exception());
      return;
    }
    if constexpr(in_place_support_v<Decoder>) {
      try {
        m_decoder->decode(m_source_buffer, out(m_source_buffer));
      } catch(const std::exception&) {
        m_writer.close(nest_current_exception(IOException("Decoder failed.")));
        return;
      }
      m_writer.write(m_source_buffer);
      reset(m_source_buffer);
    } else {
      try {
        m_decoder->decode(m_source_buffer, out(m_decoder_buffer));
      } catch(const std::exception&) {
        m_writer.close(nest_current_exception(IOException("Decoder failed.")));
        return;
      }
      m_writer.write(m_decoder_buffer);
      reset(m_decoder_buffer);
      reset(m_source_buffer);
    }
  }
}

#endif
