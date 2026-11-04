#ifndef BEAM_IO_AES_GCM_CHANNEL_HPP
#define BEAM_IO_AES_GCM_CHANNEL_HPP
#include <optional>
#include <string>
#include <utility>
#include "Beam/Codecs/AesGcmDecoder.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/IO/AesGcm.hpp"
#include "Beam/IO/AesGcmWriter.hpp"

namespace Beam {

  /** Configures the trusted public key of an AES-GCM server. */
  struct AesGcmClientConfig {

    /** The trusted server public key in PEM format. */
    std::string m_public_key;
  };

  /** Configures the private key of an AES-GCM server. */
  struct AesGcmServerConfig {

    /** The unencrypted private key in PEM format. */
    std::string m_private_key;
  };

  /**
   * Creates a server handshake context for AES-GCM channels.
   * @param config The server credentials.
   */
  inline auto make_server_context(const AesGcmServerConfig& config) {
    return Details::make_aes_gcm_context(config);
  }

  /**
   * Implements a Channel using AES-256-GCM encryption and a pinned public key
   * for server authentication.
   * @tparam C The underlying channel.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  class AesGcmChannel {
    public:

      /** The underlying channel type. */
      using Channel = dereference_t<C>;

      /** The underlying channel identifier. */
      using Identifier = typename Channel::Identifier;

      /** The underlying channel connection. */
      using Connection = typename Channel::Connection;

      /** The decrypted application reader. */
      using Reader = CodedReader<typename Channel::Reader*, AesGcmDecoder>;

      /** The encrypted application writer. */
      using Writer = AesGcmWriter<typename Channel::Writer*>;

      /**
       * Establishes a client session, closing the transport on failure.
       * @param channel An already connected channel.
       * @param config The server trust configuration.
       */
      template<Initializes<C> CF>
      AesGcmChannel(CF&& channel, const AesGcmClientConfig& config);

      /**
       * Establishes a server session, closing the transport on failure.
       * @param channel An already accepted channel.
       * @param config The server credentials.
       */
      template<Initializes<C> CF>
      AesGcmChannel(CF&& channel, const AesGcmServerConfig& config);

      ~AesGcmChannel();

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      local_ptr_t<C> m_channel;
      std::optional<Reader> m_reader;
      std::optional<Writer> m_writer;

      AesGcmChannel(const AesGcmChannel&) = delete;
      AesGcmChannel& operator =(const AesGcmChannel&) = delete;
      template<typename Config>
      void initialize(const Config& config);
  };

  template<typename C, typename Config>
  AesGcmChannel(C&&, const Config&) -> AesGcmChannel<std::remove_cvref_t<C>>;

  template<typename C> requires IsChannel<dereference_t<C>>
  template<Initializes<C> CF>
  AesGcmChannel<C>::AesGcmChannel(
      CF&& channel, const AesGcmClientConfig& config)
      : m_channel(std::forward<CF>(channel)) {
    initialize(config);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<Initializes<C> CF>
  AesGcmChannel<C>::AesGcmChannel(
      CF&& channel, const AesGcmServerConfig& config)
      : m_channel(std::forward<CF>(channel)) {
    initialize(config);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  AesGcmChannel<C>::~AesGcmChannel() {
    m_channel->get_connection().close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  const typename AesGcmChannel<C>::Identifier&
      AesGcmChannel<C>::get_identifier() const {
    return m_channel->get_identifier();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename AesGcmChannel<C>::Connection& AesGcmChannel<C>::get_connection() {
    return m_channel->get_connection();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename AesGcmChannel<C>::Reader& AesGcmChannel<C>::get_reader() {
    return *m_reader;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename AesGcmChannel<C>::Writer& AesGcmChannel<C>::get_writer() {
    return *m_writer;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<typename Config>
  void AesGcmChannel<C>::initialize(const Config& config) {
    try {
      auto material = Details::handshake_aes_gcm(*m_channel, config);
      auto sending = std::size_t(0);
      auto receiving = std::size_t(44);
      if constexpr(std::same_as<Config, AesGcmServerConfig>) {
        std::swap(sending, receiving);
      }
      m_reader.emplace(&m_channel->get_reader(),
        init(BufferView(material.data() + receiving, 32),
          BufferView(material.data() + receiving + 32, 12)));
      m_writer.emplace(&m_channel->get_writer(),
        BufferView(material.data() + sending, 32),
        BufferView(material.data() + sending + 32, 12));
      auto confirm = [&] (std::string_view expected) {
        auto buffer = SharedBuffer();
        m_reader->read(out(buffer));
        Details::require_aes_gcm(buffer == expected);
      };
      if constexpr(std::same_as<Config, AesGcmClientConfig>) {
        confirm("server ready");
        m_writer->write(from<SharedBuffer>("client ready"));
      } else {
        m_writer->write(from<SharedBuffer>("server ready"));
        confirm("client ready");
      }
    } catch(const std::exception& e) {
      m_channel->get_connection().close();
      throw_nested_with_location(ConnectException(
        "Unable to establish AES-GCM: " + std::string(e.what())));
    }
  }
}

#endif
