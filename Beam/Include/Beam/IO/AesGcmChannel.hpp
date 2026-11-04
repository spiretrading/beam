#ifndef BEAM_IO_AES_GCM_CHANNEL_HPP
#define BEAM_IO_AES_GCM_CHANNEL_HPP
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include "Beam/Codecs/AesGcmDecoder.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/IO/AesGcm.hpp"
#include "Beam/IO/AesGcmWriter.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

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

      /** Reads decrypted data after the handshake completes. */
      class Reader {
        public:
          explicit Reader(AesGcmChannel& channel) noexcept;

          bool poll() const;
          template<IsBuffer B>
          std::size_t read(Out<B> destination);
          template<IsBuffer B>
          std::size_t read(Out<B> destination, std::size_t size);

        private:
          AesGcmChannel* m_channel;
      };

      /** Writes encrypted data after the handshake completes. */
      class Writer {
        public:
          explicit Writer(AesGcmChannel& channel) noexcept;

          template<IsConstBuffer B>
          void write(const B& buffer);

        private:
          AesGcmChannel* m_channel;
      };

      /**
       * Constructs an encrypted channel to a server.
       * @param channel An already connected channel.
       * @param config The server trust configuration.
       */
      template<Initializes<C> CF>
      AesGcmChannel(CF&& channel, const AesGcmClientConfig& config);

      /**
       * Constructs an encrypted channel to a client.
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
      Reader m_reader;
      Writer m_writer;
      std::optional<CodedReader<typename Channel::Reader*, AesGcmDecoder>>
        m_decoder;
      std::optional<AesGcmWriter<typename Channel::Writer*>> m_encoder;
      mutable std::mutex m_mutex;
      std::condition_variable m_ready_condition;
      bool m_is_ready;
      std::exception_ptr m_exception;
      RoutineHandler m_handshake;

      AesGcmChannel(const AesGcmChannel&) = delete;
      AesGcmChannel& operator =(const AesGcmChannel&) = delete;
      template<typename Config>
      void initialize(const Config& config);
      void wait();
  };

  template<typename C, typename Config>
  AesGcmChannel(C&&, const Config&) -> AesGcmChannel<std::remove_cvref_t<C>>;

  template<typename C> requires IsChannel<dereference_t<C>>
  AesGcmChannel<C>::Reader::Reader(AesGcmChannel& channel) noexcept
    : m_channel(&channel) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  bool AesGcmChannel<C>::Reader::poll() const {
    auto lock = std::lock_guard(m_channel->m_mutex);
    return m_channel->m_is_ready && !m_channel->m_exception &&
      m_channel->m_decoder->poll();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsBuffer B>
  std::size_t AesGcmChannel<C>::Reader::read(Out<B> destination) {
    m_channel->wait();
    return m_channel->m_decoder->read(out(destination));
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsBuffer B>
  std::size_t AesGcmChannel<C>::Reader::read(
      Out<B> destination, std::size_t size) {
    m_channel->wait();
    return m_channel->m_decoder->read(out(destination), size);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  AesGcmChannel<C>::Writer::Writer(AesGcmChannel& channel) noexcept
    : m_channel(&channel) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsConstBuffer B>
  void AesGcmChannel<C>::Writer::write(const B& buffer) {
    m_channel->wait();
    m_channel->m_encoder->write(buffer);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<Initializes<C> CF>
  AesGcmChannel<C>::AesGcmChannel(
      CF&& channel, const AesGcmClientConfig& config)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_channel(std::forward<CF>(channel)),
        m_reader(*this),
        m_writer(*this),
        m_is_ready(false) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_handshake = spawn([=, this] { initialize(config); });
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<Initializes<C> CF>
  AesGcmChannel<C>::AesGcmChannel(
      CF&& channel, const AesGcmServerConfig& config)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_channel(std::forward<CF>(channel)),
        m_reader(*this),
        m_writer(*this),
        m_is_ready(false) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_handshake = spawn([=, this] { initialize(config); });
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  AesGcmChannel<C>::~AesGcmChannel() {
    m_channel->get_connection().close();
    m_handshake.wait();
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
    return m_reader;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename AesGcmChannel<C>::Writer& AesGcmChannel<C>::get_writer() {
    return m_writer;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<typename Config>
  void AesGcmChannel<C>::initialize(const Config& config) {
    auto exception = std::exception_ptr();
    try {
      auto material = Details::handshake_aes_gcm(*m_channel, config);
      auto sending = std::size_t(0);
      auto receiving = std::size_t(44);
      if constexpr(std::same_as<Config, AesGcmServerConfig>) {
        std::swap(sending, receiving);
      }
      m_decoder.emplace(&m_channel->get_reader(),
        init(BufferView(material.data() + receiving, 32),
          BufferView(material.data() + receiving + 32, 12)));
      m_encoder.emplace(&m_channel->get_writer(),
        BufferView(material.data() + sending, 32),
        BufferView(material.data() + sending + 32, 12));
      auto confirm = [&] (std::string_view expected) {
        auto buffer = SharedBuffer();
        m_decoder->read(out(buffer));
        Details::require_aes_gcm(buffer == expected);
      };
      if constexpr(std::same_as<Config, AesGcmClientConfig>) {
        confirm("server ready");
        m_encoder->write(from<SharedBuffer>("client ready"));
      } else {
        m_encoder->write(from<SharedBuffer>("server ready"));
        confirm("client ready");
      }
    } catch(const EndOfFileException&) {
      exception = std::current_exception();
      m_channel->get_connection().close();
    } catch(const std::exception& e) {
      exception = nest_current_exception(ConnectException(
        "Unable to establish AES-GCM: " + std::string(e.what())));
      m_channel->get_connection().close();
    }
    {
      auto lock = std::lock_guard(m_mutex);
      m_exception = std::move(exception);
      m_is_ready = true;
    }
    m_ready_condition.notify_all();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void AesGcmChannel<C>::wait() {
    auto lock = std::unique_lock(m_mutex);
    while(!m_is_ready) {
      m_ready_condition.wait(lock);
    }
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
  }
}

#endif
