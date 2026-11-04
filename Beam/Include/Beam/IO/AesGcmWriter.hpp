#ifndef BEAM_AES_GCM_WRITER_HPP
#define BEAM_AES_GCM_WRITER_HPP
#include "Beam/Codecs/AesGcmEncoder.hpp"
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/IO/BufferView.hpp"

namespace Beam {

  /**
   * Writes AES-256-GCM records, splitting buffers at the record size limit.
   * @tparam W The type of Writer to write encrypted records to.
   */
  template<typename W> requires IsWriter<dereference_t<W>>
  class AesGcmWriter {
    public:

      /** The destination to write encrypted records to. */
      using DestinationWriter = dereference_t<W>;

      /**
       * Constructs a writer for one direction of a session.
       * @param destination Initializes the destination to write to.
       * @param key The 32-byte encryption key.
       * @param nonce The 12-byte nonce base.
       */
      template<Initializes<W> WF>
      AesGcmWriter(WF&& destination,
        const IsConstBuffer auto& key, const IsConstBuffer auto& nonce);

      template<IsConstBuffer B>
      void write(const B& buffer);

    private:
      CodedWriter<W, AesGcmEncoder> m_writer;
  };

  template<typename W, IsConstBuffer K, IsConstBuffer N>
  AesGcmWriter(W&&, const K&, const N&) ->
    AesGcmWriter<std::remove_cvref_t<W>>;

  template<typename W> requires IsWriter<dereference_t<W>>
  template<Initializes<W> WF>
  AesGcmWriter<W>::AesGcmWriter(WF&& destination,
    const IsConstBuffer auto& key, const IsConstBuffer auto& nonce)
    : m_writer(std::forward<WF>(destination), init(key, nonce)) {}

  template<typename W> requires IsWriter<dereference_t<W>>
  template<IsConstBuffer B>
  void AesGcmWriter<W>::write(const B& buffer) {
    for(auto offset = std::size_t(0); offset < buffer.get_size();) {
      auto size =
        std::min(AesGcmEncoder::MAXIMUM_SIZE, buffer.get_size() - offset);
      m_writer.write(BufferView(buffer.get_data() + offset, size));
      offset += size;
    }
  }
}

#endif
