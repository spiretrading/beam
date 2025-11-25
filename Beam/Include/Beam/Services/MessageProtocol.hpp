#ifndef BEAM_MESSAGE_PROTOCOL_HPP
#define BEAM_MESSAGE_PROTOCOL_HPP
#include <utility>
#include <boost/endian.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/AsyncWriter.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SuffixBuffer.hpp"
#include "Beam/IO/ValueSpan.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleClone.hpp"
#include "Beam/Services/Message.hpp"

namespace Beam {

  /**
   * Implements a protocol used to send/receive discrete messages over a
   * Channel.
   * @param C The type of Channel to send messages to/from.
   * @param S The type of Sender used for serialization.
   * @param E The type of Encoder used.
   */
  template<typename C, IsSender S, IsEncoder E = NullEncoder> requires
    IsChannel<dereference_t<C>>
  class MessageProtocol {
    public:

      /** The type of Channel to implement the protocol for. */
      using Channel = dereference_t<C>;

      /** The type of Sender used for serialization. */
      using Sender = S;

      /** The type of Receiver used for serialization. */
      using Receiver = inverse_t<Sender>;

      /** The type of Encoder used. */
      using Encoder = E;

      /** The type of Decoder used. */
      using Decoder = inverse_t<Encoder>;

      /**
       * Constructs a MessageProtocol.
       * @param channel The Channel to adapt this protocol onto.
       * @param sender The Sender used for serialization.
       * @param receiver The Receiver used for serialization.
       * @param encoder The Encoder used.
       * @param decoder The Decoder used.
       */
      template<Initializes<C> CF, Initializes<S> SF,
        Initializes<inverse_t<S>> RF, Initializes<E> EF,
        Initializes<inverse_t<E>> DF>
      MessageProtocol(CF&& channel, SF&& sender, RF&& receiver, EF&& encoder,
        DF&& decoder);

      ~MessageProtocol();

      /**
       * Clones a value using this protocol's serializer.
       * @param value The value to clone.
       * @return A polymorphic copy of the <i>value</i> based on this protocol's
       *         serializer.
       */
      template<typename T>
      std::unique_ptr<T> clone(const T& value);

      /**
       * Encodes a message into a Buffer using this protocol.
       * @param message The message to encode.
       * @param buffer The Buffer to encode the <i>message</i> into.
       */
      template<typename T, IsBuffer B>
      void encode(const Message<T>& message, Out<B> buffer);

      /**
       * Sends a message.
       * @param message The message to send.
       */
      template<typename M> requires(!IsBuffer<M>)
      void send(const M& message);

      /**
       * Sends a Buffer.
       * @param buffer The Buffer to send.
       */
      template<IsConstBuffer B>
      void send(const B& buffer);

      /** Receives a message. */
      template<typename Message>
      Message receive();

      void close();

    private:
      mutable boost::mutex m_mutex;
      OpenState m_open_state;
      local_ptr_t<C> m_channel;
      AsyncWriter<typename Channel::Writer*> m_writer;
      Sender m_sender;
      Receiver m_receiver;
      Encoder m_encoder;
      Decoder m_decoder;
      SharedBuffer m_receive_buffer;
      SharedBuffer m_decoder_buffer;

      MessageProtocol(const MessageProtocol&) = delete;
      MessageProtocol& operator =(const MessageProtocol&) = delete;
  };

  template<typename C, typename S, typename R, typename E, typename D>
  MessageProtocol(C&&, S&&, R&&, E&&, D&&) ->
    MessageProtocol<std::remove_cvref_t<C>, std::remove_cvref_t<S>,
      std::remove_cvref_t<E>>;

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<Initializes<C> CF, Initializes<S> SF, Initializes<inverse_t<S>> RF,
    Initializes<E> EF, Initializes<inverse_t<E>> DF>
  MessageProtocol<C, S, E>::MessageProtocol(CF&& channel, SF&& sender,
    RF&& receiver, EF&& encoder, DF&& decoder)
    : m_channel(std::forward<CF>(channel)),
      m_writer(&m_channel->get_writer()),
      m_sender(std::forward<SF>(sender)),
      m_receiver(std::forward<RF>(receiver)),
      m_encoder(std::forward<EF>(encoder)),
      m_decoder(std::forward<DF>(decoder)) {}

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  MessageProtocol<C, S, E>::~MessageProtocol() {
    close();
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<typename T>
  std::unique_ptr<T> MessageProtocol<C, S, E>::clone(const T& value) {
    auto lock = boost::lock_guard(m_mutex);
    return shuttle_clone(value, m_sender, m_receiver);
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<typename T, IsBuffer B>
  void MessageProtocol<C, S, E>::encode(
      const Message<T>& message, Out<B> buffer) {
    append(*buffer, std::uint32_t(0));
    auto serialization_buffer = B();
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender.set(Ref(serialization_buffer));
      m_sender.send(&message);
    }
    auto encoder_buffer = SuffixBuffer(Ref(*buffer), sizeof(std::uint32_t));
    auto size = m_encoder.encode(serialization_buffer, out(encoder_buffer));
    write(*buffer, 0, boost::endian::native_to_little<std::uint32_t>(size));
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<typename M> requires(!IsBuffer<M>)
  void MessageProtocol<C, S, E>::send(const M& message) {
    m_open_state.ensure_open();
    auto sender_buffer = SharedBuffer();
    auto encoder_buffer = SharedBuffer();
    if(in_place_support_v<Encoder>) {
      append(sender_buffer, std::uint32_t(0));
    } else {
      append(encoder_buffer, std::uint32_t(0));
    }
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender.set(Ref(sender_buffer));
      m_sender.send(message);
    }
    if(in_place_support_v<Encoder>) {
      auto sender_view_buffer =
        SuffixBuffer(Ref(sender_buffer), sizeof(std::uint32_t));
      auto size = m_encoder.encode(sender_view_buffer, out(sender_view_buffer));
      write(
        sender_buffer, 0, boost::endian::native_to_little<std::uint32_t>(size));
      m_writer.write(sender_buffer);
    } else {
      auto encoder_view_buffer =
        SuffixBuffer(Ref(encoder_buffer), sizeof(std::uint32_t));
      auto size = m_encoder.encode(sender_buffer, out(encoder_view_buffer));
      write(encoder_buffer, 0,
        boost::endian::native_to_little<std::uint32_t>(size));
      m_writer.write(encoder_buffer);
    }
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<IsConstBuffer B>
  void MessageProtocol<C, S, E>::send(const B& buffer) {
    m_open_state.ensure_open();
    m_writer.write(buffer);
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  template<typename Message>
  Message MessageProtocol<C, S, E>::receive() {
    try {
      auto size = std::uint32_t(0);
      auto span = ValueSpan(Ref(size));
      reset(span);
      while(span.get_size() != sizeof(size)) {
        m_channel->get_reader().read(out(span), sizeof(size) - span.get_size());
      }
      size = boost::endian::little_to_native<std::uint32_t>(size);
      while(size > m_receive_buffer.get_size()) {
        m_channel->get_reader().read(out(m_receive_buffer),
          size - m_receive_buffer.get_size());
      }
      if(in_place_support_v<Decoder>) {
        m_decoder.decode(m_receive_buffer, out(m_receive_buffer));
        m_receiver.set(Ref(m_receive_buffer));
      } else {
        m_decoder.decode(m_receive_buffer, out(m_decoder_buffer));
        m_receiver.set(Ref(m_decoder_buffer));
      }
      auto message = Message();
      m_receiver.shuttle(message);
      reset(m_receive_buffer);
      if(!in_place_support_v<Decoder>) {
        reset(m_decoder_buffer);
      }
      return message;
    } catch(const std::exception&) {
      reset(m_receive_buffer);
      reset(m_decoder_buffer);
      throw;
    }
  }

  template<typename C, IsSender S, IsEncoder E> requires
    IsChannel<dereference_t<C>>
  void MessageProtocol<C, S, E>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
