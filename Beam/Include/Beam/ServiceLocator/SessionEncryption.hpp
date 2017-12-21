#ifndef BEAM_SESSION_ENCRYPTION_HPP
#define BEAM_SESSION_ENCRYPTION_HPP
#include <string>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Details {
  inline CryptoPP::AutoSeededRandomPool& GetRandomPool() {
    static CryptoPP::AutoSeededRandomPool randomPool;
    return randomPool;
  }
}

  //! The length of a session id.
  static const unsigned int SESSION_ID_LENGTH = 32;

  //! Generates a session id.
  inline std::string GenerateSessionId() {
    char randomBytes[SESSION_ID_LENGTH];
    Details::GetRandomPool().GenerateBlock(reinterpret_cast<CryptoPP::byte*>(
      randomBytes), SESSION_ID_LENGTH);
    for(int i = 0; i < SESSION_ID_LENGTH; ++i) {
      randomBytes[i] = (static_cast<unsigned char>(randomBytes[i]) % 26) + 'a';
    }
    return std::string(randomBytes, SESSION_ID_LENGTH);
  }

  //! Computes the hexadecimal SHA hash-code of a given string.
  /*!
    \param source The source string to hash.
    \return An uppercase, hexadecimal SHA hash-code of the <i>source</i>.
  */
  inline std::string ComputeSHA(const std::string& source) {
    CryptoPP::SHA1 sha;
    CryptoPP::byte digest[CryptoPP::SHA1::DIGESTSIZE];
    sha.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte*>(
      source.c_str()), source.length());
    CryptoPP::HexEncoder encoder;
    std::string output;
    encoder.Attach(new CryptoPP::StringSink(output));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    return output;
  }

  //! Generates an encryption key to use with a session id.
  inline unsigned int GenerateEncryptionKey() {
    unsigned int randomBytes;
    Details::GetRandomPool().GenerateBlock(
      reinterpret_cast<CryptoPP::byte*>(&randomBytes), sizeof(randomBytes));
    return randomBytes;
  }
}
}

#endif
