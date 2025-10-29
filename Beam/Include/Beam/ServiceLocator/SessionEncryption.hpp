#ifndef BEAM_SESSION_ENCRYPTION_HPP
#define BEAM_SESSION_ENCRYPTION_HPP
#include <array>
#include <string>
#include <string_view>
#include <type_traits>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include "Beam/Utilities/Bcrypt.hpp"

namespace Beam {

  /** The length of a session id. */
  static constexpr auto SESSION_ID_LENGTH = 32U;

  /** Generates a session id. */
  inline std::string generate_session_id() {
    auto random_bytes = std::array<char, SESSION_ID_LENGTH>();
    Details::get_random_pool().GenerateBlock(reinterpret_cast<CryptoPP::byte*>(
      random_bytes.data()), SESSION_ID_LENGTH);
    for(auto i = 0U; i < SESSION_ID_LENGTH; ++i) {
      random_bytes[i] =
        (static_cast<unsigned char>(random_bytes[i]) % 26) + 'a';
    }
    return std::string(random_bytes.data(), SESSION_ID_LENGTH);
  }

  /**
   * Computes the hexadecimal SHA hash-code of a given string.
   * @param source The source string to hash.
   * @return An uppercase, hexadecimal SHA hash-code of the <i>source</i>.
   */
  inline std::string compute_sha(std::string_view source) {
    auto sha = CryptoPP::SHA1();
    auto digest = std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE>();
    sha.CalculateDigest(digest.data(), reinterpret_cast<const CryptoPP::byte*>(
      source.data()), source.length());
    auto encoder = CryptoPP::HexEncoder();
    auto output = std::string();
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(digest.data(), CryptoPP::SHA1::DIGESTSIZE);
    encoder.MessageEnd();
    return output;
  }

  /** Generates an encryption key to use with a session id. */
  inline unsigned int generate_encryption_key() {
    auto random_bytes = std::make_unsigned_t<int>();
    Details::get_random_pool().GenerateBlock(
      reinterpret_cast<CryptoPP::byte*>(&random_bytes), sizeof(random_bytes));
    return random_bytes;
  }
}

#endif
