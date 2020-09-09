#ifndef BEAM_MESSAGE_PROTOCOL_HPP
#define BEAM_MESSAGE_PROTOCOL_HPP
#include <utility>
#include <boost/thread/mutex.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/AsyncWriter.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/BufferView.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleClone.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam::Services {

  /**
   * Implements a protocol used to send/receive discrete messages over a
   * Channel.
   * @param C The type of Channel to send messages to/from.
   * @param S The type of Sender used for serialization.
   * @param E The type of Encoder used.
   */
  template<typename C, typename S, typename E = Codecs::NullEncoder>
  class MessageProtocol {
    public:

      /** The type of Channel to implement the protocol for. */
      using Channel = GetTryDereferenceType<C>;

      /** The type of Sender used for serialization. */
      using Sender = S;

      /** The type of Receiver used for serialization. */
      using Receiver = Serialization::GetInverse<Sender>;

      /** The type of Encoder used. */
      using Encoder = E;

      /** The type of Decoder used. */
      using Decoder = Codecs::GetInverse<Encoder>;

      /**
       * Constructs a MessageProtocol.
       * @param channel The Channel to adapt this protocol onto.
       * @param sender The Sender used for serialization.
       * @param receiver The Receiver used for serialization.
       * @param encoder The Encoder used.
       * @param decoder The Decoder used.
       */
      template<typename CF, typename SF, typename RF, typename EF, typename DF>
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
      std::unique_ptr<T> Clone(const T& value);

      /**
       * Encodes a message into a Buffer using this protocol.
       * @param message The message to encode.
       * @param buffer The Buffer to encode the <i>message</i> into.
       */
      template<typename Message, typename Buffer>
      void Encode(const Message& message, Out<Buffer> buffer);

      /**
       * Sends a message.
       * @param message The message to send.
       */
      template<typename Message>
      std::enable_if_t<!ImplementsConcept<Message, IO::Buffer>::value> Send(
        const Message& message);

      /**
       * Sends a Buffer.
       * @param buffer The Buffer to send.
       */
      template<typename Buffer>
      std::enable_if_t<ImplementsConcept<Buffer, IO::Buffer>::value> Send(
        const Buffer& buffer);

      /** Receives a message. */
      template<typename Message>
      Message Receive();

      void Close();

    private:
      mutable boost::mutex m_mutex;
      IO::OpenState m_openState;
      GetOptionalLocalPtr<C> m_channel;
      IO::AsyncWriter<typename Channel::Writer*> m_writer;
      LocalPtr<Sender> m_sender;
      LocalPtr<Receiver> m_receiver;
      LocalPtr<Encoder> m_encoder;
      LocalPtr<Decoder> m_decoder;
      typename Channel::Reader::Buffer m_receiveBuffer;
      typename Channel::Reader::Buffer m_decoderBuffer;
  };

  template<typename C, typename S, typename E>
  template<typename CF, typename SF, typename RF, typename EF, typename DF>
  MessageProtocol<C, S, E>::MessageProtocol(CF&& channel, SF&& sender,
    RF&& receiver, EF&& encoder, DF&& decoder)
    : m_channel(std::forward<CF>(channel)),
      m_writer(&m_channel->GetWriter()),
      m_sender(std::forward<SF>(sender)),
      m_receiver(std::forward<RF>(receiver)),
      m_encoder(std::forward<EF>(encoder)),
      m_decoder(std::forward<DF>(decoder)) {}

  template<typename C, typename S, typename E>
  MessageProtocol<C, S, E>::~MessageProtocol() {
    Close();
  }

  template<typename C, typename S, typename E>
  template<typename T>
  std::unique_ptr<T> MessageProtocol<C, S, E>::Clone(const T& value) {
    auto lock = boost::lock_guard(m_mutex);
    return Serialization::ShuttleClone(value, *m_sender, *m_receiver);
  }

  template<typename C, typename S, typename E>
  template<typename Message, typename Buffer>
  void MessageProtocol<C, S, E>::Encode(const Message& message,
      Out<Buffer> buffer) {
    buffer->Append(std::uint32_t(0));
    auto serializationBuffer = Buffer();
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender->SetSink(Ref(serializationBuffer));
      m_sender->Send(message);
    }
    auto encoderViewBuffer = IO::BufferView(Ref(*buffer),
      sizeof(std::uint32_t));
    auto size = m_encoder->Encode(serializationBuffer,
      Store(encoderViewBuffer));
    buffer->Write(0, ToLittleEndian<std::uint32_t>(size));
  }

  template<typename C, typename S, typename E>
  template<typename Message>
  std::enable_if_t<!ImplementsConcept<Message, IO::Buffer>::value>
      MessageProtocol<C, S, E>::Send(const Message& message) {
    m_openState.EnsureOpen();
    auto senderBuffer = typename Channel::Writer::Buffer();
    auto encoderBuffer = typename Channel::Writer::Buffer();
    if(Codecs::InPlaceSupport<Encoder>::value) {
      senderBuffer.Append(std::uint32_t(0));
    } else {
      encoderBuffer.Append(std::uint32_t(0));
    }
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender->SetSink(Ref(senderBuffer));
      m_sender->Send(message);
    }
    if(Codecs::InPlaceSupport<Encoder>::value) {
      auto senderViewBuffer = IO::BufferView(Ref(senderBuffer),
        sizeof(std::uint32_t));
      auto size = m_encoder->Encode(senderViewBuffer, Store(senderViewBuffer));
      senderBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
      m_writer.Write(senderBuffer);
    } else {
      auto encoderViewBuffer = IO::BufferView(Ref(encoderBuffer),
        sizeof(std::uint32_t));
      auto size = m_encoder->Encode(senderBuffer, Store(encoderViewBuffer));
      encoderBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
      m_writer.Write(encoderBuffer);
    }
  }

  template<typename C, typename S, typename E>
  template<typename Buffer>
  std::enable_if_t<ImplementsConcept<Buffer, IO::Buffer>::value>
      MessageProtocol<C, S, E>::Send(const Buffer& buffer) {
    m_openState.EnsureOpen();
    m_writer.Write(buffer);
  }

  template<typename C, typename S, typename E>
  template<typename Message>
  Message MessageProtocol<C, S, E>::Receive() {
    try {
      auto size = std::uint32_t(0);
      auto remainingSizeRead = sizeof(std::uint32_t);
      while(remainingSizeRead != 0) {
        remainingSizeRead -= m_channel->GetReader().Read(
          reinterpret_cast<char*>(&size) +
          (sizeof(std::uint32_t) - remainingSizeRead), remainingSizeRead);
      }
      size = FromLittleEndian<std::uint32_t>(size);
      while(size > m_receiveBuffer.GetSize()) {
        m_channel->GetReader().Read(Store(m_receiveBuffer),
          size - m_receiveBuffer.GetSize());
      }
      if(Codecs::InPlaceSupport<Decoder>::value) {
        m_decoder->Decode(m_receiveBuffer, Store(m_receiveBuffer));
        m_receiver->SetSource(Ref(m_receiveBuffer));
      } else {
        m_decoder->Decode(m_receiveBuffer, Store(m_decoderBuffer));
        m_receiver->SetSource(Ref(m_decoderBuffer));
      }
      auto message = Message();
      m_receiver->Shuttle(message);
      m_receiveBuffer.Reset();
      if(!Codecs::InPlaceSupport<Decoder>::value) {
        m_decoderBuffer.Reset();
      }
      return message;
    } catch(const std::exception&) {
      m_receiveBuffer.Reset();
      m_decoderBuffer.Reset();
      BOOST_RETHROW;
    }
  }

  template<typename C, typename S, typename E>
  void MessageProtocol<C, S, E>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }
}

#endif
