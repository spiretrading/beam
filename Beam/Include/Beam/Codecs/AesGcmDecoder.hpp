#ifndef BEAM_AES_GCM_DECODER_HPP
#define BEAM_AES_GCM_DECODER_HPP
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include "Beam/Codecs/AesGcmEncoder.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /** Authenticates and decrypts ordered AES-256-GCM records. */
  class AesGcmDecoder {
    public:

      /** The maximum plaintext record size. */
      static constexpr auto MAXIMUM_SIZE = AesGcmEncoder::MAXIMUM_SIZE;

      /** The authentication tag size. */
      static constexpr auto TAG_SIZE = AesGcmEncoder::TAG_SIZE;

      /** The maximum number of records per key and nonce base pair. */
      static constexpr auto MAXIMUM_RECORDS = AesGcmEncoder::MAXIMUM_RECORDS;

      /**
       * Constructs a decoder for one direction of a session.
       * @param key The 32-byte decryption key.
       * @param nonce The 12-byte nonce base.
       */
      AesGcmDecoder(
        const IsConstBuffer auto& key, const IsConstBuffer auto& nonce);

      template<IsConstBuffer S, IsBuffer B>
      std::size_t decode(const S& source, Out<B> destination);

    private:
      CryptoPP::GCM<CryptoPP::AES>::Decryption m_cipher;
      CryptoPP::SecByteBlock m_nonce;
      std::uint64_t m_sequence;

      AesGcmDecoder(const AesGcmDecoder&) = delete;
      AesGcmDecoder& operator =(const AesGcmDecoder&) = delete;
  };

  template<>
  struct inverse<AesGcmDecoder> {
    using type = AesGcmEncoder;
  };

  AesGcmDecoder::AesGcmDecoder(
      const IsConstBuffer auto& key, const IsConstBuffer auto& nonce)
      : m_nonce(AesGcmEncoder::NONCE_SIZE),
        m_sequence(0) {
    if(key.get_size() != AesGcmEncoder::KEY_SIZE ||
        nonce.get_size() != m_nonce.size()) {
      boost::throw_with_location(
        DecoderException("Invalid AES-GCM key/nonce."));
    }
    std::memcpy(m_nonce.data(), nonce.get_data(), m_nonce.size());
    m_cipher.SetKeyWithIV(
      reinterpret_cast<const CryptoPP::byte*>(key.get_data()), key.get_size(),
      m_nonce.data(), m_nonce.size());
  }

  template<IsConstBuffer S, IsBuffer B>
  std::size_t AesGcmDecoder::decode(const S& source, Out<B> destination) {
    if(source.get_size() < TAG_SIZE ||
        source.get_size() > MAXIMUM_SIZE + TAG_SIZE ||
        m_sequence == MAXIMUM_RECORDS) {
      m_sequence = MAXIMUM_RECORDS;
      boost::throw_with_location(DecoderException("Invalid AES-GCM record."));
    }
    auto size = source.get_size() - TAG_SIZE;
    auto plaintext = CryptoPP::SecByteBlock(std::max(size, std::size_t(1)));
    auto nonce = AesGcmEncoder::make_nonce(m_nonce, m_sequence);
    try {
      auto input = reinterpret_cast<const CryptoPP::byte*>(source.get_data());
      if(!m_cipher.DecryptAndVerify(plaintext.data(), input + size, TAG_SIZE,
          nonce.data(), static_cast<int>(nonce.size()), nullptr, 0, input,
          size)) {
        m_sequence = MAXIMUM_RECORDS;
        boost::throw_with_location(
          DecoderException("AES-GCM authentication failed."));
      }
    } catch(const CryptoPP::Exception&) {
      m_sequence = MAXIMUM_RECORDS;
      throw_nested_with_location(
        DecoderException("AES-GCM decryption failed."));
    }
    if(reserve(*destination, size) < size) {
      boost::throw_with_location(DecoderException("Insufficient space."));
    }
    if(size != 0) {
      destination->write(0, plaintext.data(), size);
    }
    destination->shrink(destination->get_size() - size);
    ++m_sequence;
    return size;
  }
}

#endif
