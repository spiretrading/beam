#ifndef BEAM_AES_GCM_ENCODER_HPP
#define BEAM_AES_GCM_ENCODER_HPP
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <cryptopp/aes.h>
#include <cryptopp/gcm.h>
#include <cryptopp/secblock.h>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
  class AesGcmDecoder;

  /** Encrypts ordered records using AES-256-GCM. */
  class AesGcmEncoder {
    public:

      /** The encryption key size. */
      static constexpr auto KEY_SIZE = std::size_t(32);

      /** The nonce base size. */
      static constexpr auto NONCE_SIZE = std::size_t(12);

      /** The maximum plaintext record size. */
      static constexpr auto MAXIMUM_SIZE = std::size_t(16 * 1024);

      /** The authentication tag size. */
      static constexpr auto TAG_SIZE = std::size_t(16);

      /** The maximum number of records per key and nonce base pair. */
      static constexpr auto MAXIMUM_RECORDS = std::uint64_t(1) << 24;

      /**
       * Constructs an encoder for one direction of a session.
       * @param key The 32-byte encryption key.
       * @param nonce The 12-byte nonce base.
       */
      AesGcmEncoder(
        const IsConstBuffer auto& key, const IsConstBuffer auto& nonce);

      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);

    private:
      friend class AesGcmDecoder;
      CryptoPP::GCM<CryptoPP::AES>::Encryption m_cipher;
      CryptoPP::SecByteBlock m_nonce;
      std::uint64_t m_sequence;

      static std::array<CryptoPP::byte, NONCE_SIZE> make_nonce(
        const CryptoPP::SecByteBlock& base, std::uint64_t sequence);
      AesGcmEncoder(const AesGcmEncoder&) = delete;
      AesGcmEncoder& operator =(const AesGcmEncoder&) = delete;
  };

  template<>
  struct inverse<AesGcmEncoder> {
    using type = AesGcmDecoder;
  };

  AesGcmEncoder::AesGcmEncoder(
      const IsConstBuffer auto& key, const IsConstBuffer auto& nonce)
      : m_nonce(NONCE_SIZE),
        m_sequence(0) {
    if(key.get_size() != KEY_SIZE || nonce.get_size() != m_nonce.size()) {
      boost::throw_with_location(
        EncoderException("Invalid AES-GCM key/nonce."));
    }
    std::memcpy(m_nonce.data(), nonce.get_data(), m_nonce.size());
    m_cipher.SetKeyWithIV(
      reinterpret_cast<const CryptoPP::byte*>(key.get_data()), key.get_size(),
      m_nonce.data(), m_nonce.size());
  }

  template<IsConstBuffer S, IsBuffer B>
  std::size_t AesGcmEncoder::encode(const S& source, Out<B> destination) {
    if(source.get_size() > MAXIMUM_SIZE ||
        m_sequence == MAXIMUM_RECORDS) {
      boost::throw_with_location(EncoderException("AES-GCM limit exceeded."));
    }
    auto size = source.get_size() + TAG_SIZE;
    if(reserve(*destination, size) < size) {
      boost::throw_with_location(EncoderException("Insufficient space."));
    }
    auto nonce = make_nonce(m_nonce, m_sequence);
    ++m_sequence;
    try {
      auto output =
        reinterpret_cast<CryptoPP::byte*>(destination->get_mutable_data());
      m_cipher.EncryptAndAuthenticate(output, output + source.get_size(),
        TAG_SIZE, nonce.data(), static_cast<int>(nonce.size()), nullptr, 0,
        reinterpret_cast<const CryptoPP::byte*>(source.get_data()),
        source.get_size());
    } catch(const CryptoPP::Exception&) {
      m_sequence = MAXIMUM_RECORDS;
      throw_nested_with_location(
        EncoderException("AES-GCM encryption failed."));
    }
    destination->shrink(destination->get_size() - size);
    return size;
  }

  inline std::array<CryptoPP::byte, AesGcmEncoder::NONCE_SIZE>
      AesGcmEncoder::make_nonce(
        const CryptoPP::SecByteBlock& base, std::uint64_t sequence) {
    auto nonce = std::array<CryptoPP::byte, NONCE_SIZE>();
    std::ranges::copy(base, nonce.begin());
    for(auto i = std::size_t(0); i != sizeof(sequence); ++i) {
      nonce[nonce.size() - 1 - i] ^= static_cast<CryptoPP::byte>(sequence);
      sequence >>= 8;
    }
    return nonce;
  }
}

#endif
