#ifndef BEAM_SESSION_ENCRYPTION_HPP
#define BEAM_SESSION_ENCRYPTION_HPP
#include <array>
#include <string>
#include <type_traits>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {
namespace Details {
  inline CryptoPP::AutoSeededRandomPool& GetRandomPool() {
    static auto randomPool = CryptoPP::AutoSeededRandomPool();
    return randomPool;
  }
}

  /** The length of a session id. */
  static constexpr auto SESSION_ID_LENGTH = 32U;

  /** Generates a session id. */
  inline std::string GenerateSessionId() {
    auto randomBytes = std::array<char, SESSION_ID_LENGTH>();
    Details::GetRandomPool().GenerateBlock(reinterpret_cast<CryptoPP::byte*>(
      randomBytes.data()), SESSION_ID_LENGTH);
    for(auto i = 0U; i < SESSION_ID_LENGTH; ++i) {
      randomBytes[i] = (static_cast<unsigned char>(randomBytes[i]) % 26) + 'a';
    }
    return std::string(randomBytes.data(), SESSION_ID_LENGTH);
  }

  /**
   * Computes the hexadecimal SHA hash-code of a given string.
   * @param source The source string to hash.
   * @return An uppercase, hexadecimal SHA hash-code of the <i>source</i>.
   */
  inline std::string ComputeSHA(const std::string& source) {
    auto sha = CryptoPP::SHA1();
    auto digest = std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE>();
    sha.CalculateDigest(digest.data(), reinterpret_cast<const CryptoPP::byte*>(
      source.c_str()), source.length());
    auto encoder = CryptoPP::HexEncoder();
    auto output = std::string();
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(digest.data(), CryptoPP::SHA1::DIGESTSIZE);
    encoder.MessageEnd();
    return output;
  }

  /** Generates an encryption key to use with a session id. */
  inline unsigned int GenerateEncryptionKey() {
    auto randomBytes = std::make_unsigned_t<int>();
    Details::GetRandomPool().GenerateBlock(
      reinterpret_cast<CryptoPP::byte*>(&randomBytes), sizeof(randomBytes));
    return randomBytes;
  }
}

#endif
