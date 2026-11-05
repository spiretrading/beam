#ifndef BEAM_IO_AES_GCM_HPP
#define BEAM_IO_AES_GCM_HPP
#include <array>
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <cryptopp/secblock.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/SharedBuffer.hpp"

namespace Beam {
  struct AesGcmClientConfig;
  struct AesGcmServerConfig;

namespace Details {
  using AesGcmSslContext = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;
  using AesGcmSsl = std::unique_ptr<SSL, decltype(&SSL_free)>;
  using AesGcmBio = std::unique_ptr<BIO, decltype(&BIO_free)>;
  using AesGcmKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

  inline void require_aes_gcm(auto result) {
    if(result != 1) {
      auto error = ERR_get_error();
      auto message = std::array<char, 256>();
      ERR_error_string_n(error, message.data(), message.size());
      boost::throw_with_location(ConnectException(
        "AES-GCM handshake failed: " + std::string(message.data())));
    }
  }

  inline AesGcmBio make_aes_gcm_bio(const std::string& text) {
    if(!std::in_range<int>(text.size())) {
      boost::throw_with_location(
        ConnectException("Key size exceeds supported range."));
    }
    auto bio = AesGcmBio(
      BIO_new_mem_buf(text.data(), static_cast<int>(text.size())), &BIO_free);
    require_aes_gcm(static_cast<bool>(bio));
    return bio;
  }

  inline int select_aes_gcm_protocol(SSL*, const unsigned char** output,
      unsigned char* size, const unsigned char* input, unsigned int length,
      void*) {
    static constexpr auto PROTOCOL = "\x0e" "beam-aes-gcm/1";
    auto result = SSL_select_next_proto(
      const_cast<unsigned char**>(output), size,
      reinterpret_cast<const unsigned char*>(PROTOCOL), 15, input, length);
    if(result != OPENSSL_NPN_NEGOTIATED) {
      return SSL_TLSEXT_ERR_ALERT_FATAL;
    }
    return SSL_TLSEXT_ERR_OK;
  }

  inline AesGcmSslContext make_aes_gcm_context() {
    auto context = AesGcmSslContext(SSL_CTX_new(TLS_method()), &SSL_CTX_free);
    require_aes_gcm(static_cast<bool>(context));
    require_aes_gcm(
      SSL_CTX_set_min_proto_version(context.get(), TLS1_3_VERSION));
    require_aes_gcm(
      SSL_CTX_set_max_proto_version(context.get(), TLS1_3_VERSION));
    require_aes_gcm(
      SSL_CTX_set_ciphersuites(context.get(), "TLS_AES_256_GCM_SHA384"));
    require_aes_gcm(SSL_CTX_set_num_tickets(context.get(), 0));
    SSL_CTX_set_session_cache_mode(context.get(), SSL_SESS_CACHE_OFF);
    auto type = static_cast<unsigned char>(TLSEXT_cert_type_rpk);
    require_aes_gcm(SSL_CTX_set1_server_cert_type(context.get(), &type, 1));
    return context;
  }

  AesGcmSslContext make_aes_gcm_context(
      const std::same_as<AesGcmClientConfig> auto& config) {
    if(config.m_public_key.empty()) {
      boost::throw_with_location(
        ConnectException("Missing server public key."));
    }
    return make_aes_gcm_context();
  }

  AesGcmSslContext make_aes_gcm_context(
      const std::same_as<AesGcmServerConfig> auto& config) {
    auto context = make_aes_gcm_context();
    auto bio = make_aes_gcm_bio(config.m_private_key);
    auto key = AesGcmKey(PEM_read_bio_PrivateKey(bio.get(), nullptr,
      [] (char*, int, int, void*) { return 0; }, nullptr), &EVP_PKEY_free);
    require_aes_gcm(static_cast<bool>(key));
    require_aes_gcm(SSL_CTX_use_PrivateKey(context.get(), key.get()));
    SSL_CTX_set_alpn_select_cb(
      context.get(), &select_aes_gcm_protocol, nullptr);
    return context;
  }

  void verify_aes_gcm_peer(
      SSL& session, const std::same_as<AesGcmClientConfig> auto& config) {
    auto peer = SSL_get0_peer_rpk(&session);
    require_aes_gcm(static_cast<bool>(peer));
    auto bio = make_aes_gcm_bio(config.m_public_key);
    auto key = AesGcmKey(PEM_read_bio_PUBKEY(
      bio.get(), nullptr, nullptr, nullptr), &EVP_PKEY_free);
    require_aes_gcm(static_cast<bool>(key));
    require_aes_gcm(EVP_PKEY_eq(key.get(), peer));
  }

  template<IsChannel C, typename Config>
  CryptoPP::SecByteBlock handshake_aes_gcm(C& channel, const Config& config) {
    auto context = make_aes_gcm_context(config);
    auto session = AesGcmSsl(SSL_new(context.get()), &SSL_free);
    require_aes_gcm(static_cast<bool>(session));
    auto incoming = AesGcmBio(BIO_new(BIO_s_mem()), &BIO_free);
    auto outgoing = AesGcmBio(BIO_new(BIO_s_mem()), &BIO_free);
    require_aes_gcm(static_cast<bool>(incoming) && static_cast<bool>(outgoing));
    SSL_set_bio(session.get(), incoming.release(), outgoing.release());
    if constexpr(std::same_as<Config, AesGcmClientConfig>) {
      SSL_set_connect_state(session.get());
      static constexpr auto PROTOCOL = "\x0e" "beam-aes-gcm/1";
      require_aes_gcm(SSL_set_alpn_protos(session.get(),
        reinterpret_cast<const unsigned char*>(PROTOCOL), 15) == 0);
    } else {
      SSL_set_accept_state(session.get());
    }
    auto& reader = channel.get_reader();
    auto& writer = channel.get_writer();
    for(auto i = 0; i != 64; ++i) {
      ERR_clear_error();
      auto result = SSL_do_handshake(session.get());
      auto error = SSL_get_error(session.get(), result);
      auto pending = BIO_ctrl_pending(SSL_get_wbio(session.get()));
      if(pending != 0) {
        require_aes_gcm(std::in_range<int>(pending));
        auto buffer = SharedBuffer(pending);
        require_aes_gcm(BIO_read(SSL_get_wbio(session.get()),
          buffer.get_mutable_data(), static_cast<int>(pending)) ==
            static_cast<int>(pending));
        writer.write(buffer);
      }
      if(result == 1) {
        require_aes_gcm(SSL_get_negotiated_server_cert_type(session.get()) ==
          TLSEXT_cert_type_rpk);
        if constexpr(std::same_as<Config, AesGcmClientConfig>) {
          verify_aes_gcm_peer(*session, config);
        }
        auto protocol = static_cast<const unsigned char*>(nullptr);
        auto length = 0U;
        SSL_get0_alpn_selected(session.get(), &protocol, &length);
        require_aes_gcm(std::string_view(
          reinterpret_cast<const char*>(protocol), length) == "beam-aes-gcm/1");
        auto material = CryptoPP::SecByteBlock(88);
        static constexpr auto LABEL = "EXPORTER-Beam-AesGcmChannel-v1";
        require_aes_gcm(SSL_export_keying_material(session.get(),
          material.data(), material.size(), LABEL, sizeof(LABEL) - 1, nullptr,
          0, 1));
        return material;
      }
      require_aes_gcm(error == SSL_ERROR_WANT_READ);
      auto buffer = SharedBuffer();
      reader.read(out(buffer));
      require_aes_gcm(std::in_range<int>(buffer.get_size()));
      require_aes_gcm(BIO_write(SSL_get_rbio(session.get()), buffer.get_data(),
        static_cast<int>(buffer.get_size())) ==
          static_cast<int>(buffer.get_size()));
    }
    boost::throw_with_location(ConnectException("Handshake limit exceeded."));
  }
}
}

#endif
