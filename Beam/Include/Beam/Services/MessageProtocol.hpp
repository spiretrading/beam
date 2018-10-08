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
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/ShuttleClone.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam::Services {

  /** Implements a protocol used to send/receive discrete messages over a
      Channel.
      \tparam ChannelType The type of Channel to send messages to/from.
      \tparam SenderType The type of Sender used for serialization.
      \tparam EncoderType The type of Encoder used.
   */
  template<typename ChannelType, typename SenderType,
    typename EncoderType = Codecs::NullEncoder>
  class MessageProtocol {
    public:

      //! The type of Channel to implement the protocol for.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! The type of Sender used for serialization.
      using Sender = SenderType;

      //! The type of Receiver used for serialization.
      using Receiver = Serialization::GetInverse<Sender>;

      //! The type of Encoder used.
      using Encoder = EncoderType;

      //! The type of Decoder used.
      using Decoder = Codecs::GetInverse<Encoder>;

      //! Constructs a MessageProtocol.
      /*!
        \param channel The Channel to adapt this protocol onto.
        \param sender The Sender used for serialization.
        \param receiver The Receiver used for serialization.
        \param encoder The Encoder used.
        \param decoder The Decoder used.
      */
      template<typename ChannelForward, typename SenderForward,
        typename ReceiverForward, typename EncoderForward,
        typename DecoderForward>
      MessageProtocol(ChannelForward&& channel, SenderForward&& sender,
        ReceiverForward&& receiver, EncoderForward&& encoder,
        DecoderForward&& decoder);

      //! Returns the Channel.
      const Channel& GetChannel() const;

      //! Returns the Channel.
      Channel& GetChannel();

      //! Clones a value using this protocol's serializer.
      /*!
        \param value The value to clone.
        \return A polymorphic copy of the <i>value</i> based on this protocol's
                serializer.
      */
      template<typename T>
      std::unique_ptr<T> Clone(const T& value);

      //! Encodes a message into a Buffer using this protocol.
      /*!
        \param message The message to encode.
        \param buffer The Buffer to encode the <i>message</i> into.
      */
      template<typename Message, typename Buffer>
      void Encode(const Message& message, Out<Buffer> buffer);

      //! Sends a message.
      /*!
        \param message The message to send.
      */
      template<typename Message>
      typename std::enable_if<!ImplementsConcept<
        Message, IO::Buffer>::value>::type Send(const Message& message);

      //! Sends a Buffer.
      /*!
        \param buffer The Buffer to send.
      */
      template<typename Buffer>
      typename std::enable_if<ImplementsConcept<
        Buffer, IO::Buffer>::value>::type Send(const Buffer& buffer);

      //! Receives a message.
      template<typename Message>
      Message Receive();

    private:
      mutable boost::mutex m_mutex;
      GetOptionalLocalPtr<ChannelType> m_channel;
      IO::AsyncWriter<typename Channel::Writer*> m_writer;
      LocalPtr<Sender> m_sender;
      LocalPtr<Receiver> m_receiver;
      LocalPtr<Encoder> m_encoder;
      LocalPtr<Decoder> m_decoder;
      typename Channel::Reader::Buffer m_receiveBuffer;
      typename Channel::Reader::Buffer m_decoderBuffer;
  };

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename ChannelForward, typename SenderForward,
    typename ReceiverForward, typename EncoderForward, typename DecoderForward>
  MessageProtocol<ChannelType, SenderType, EncoderType>::MessageProtocol(
      ChannelForward&& channel, SenderForward&& sender,
      ReceiverForward&& receiver, EncoderForward&& encoder,
      DecoderForward&& decoder)
      : m_channel(std::forward<ChannelForward>(channel)),
        m_writer(&m_channel->GetWriter()),
        m_sender(std::forward<SenderForward>(sender)),
        m_receiver(std::forward<ReceiverForward>(receiver)),
        m_encoder(std::forward<EncoderForward>(encoder)),
        m_decoder(std::forward<DecoderForward>(decoder)) {}

  template<typename ChannelType, typename SenderType, typename EncoderType>
  const typename MessageProtocol<ChannelType, SenderType, EncoderType>::Channel&
      MessageProtocol<ChannelType, SenderType, EncoderType>::
      GetChannel() const {
    return *m_channel;
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  typename MessageProtocol<ChannelType, SenderType, EncoderType>::Channel&
      MessageProtocol<ChannelType, SenderType, EncoderType>::GetChannel() {
    return *m_channel;
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename T>
  std::unique_ptr<T> MessageProtocol<ChannelType, SenderType, EncoderType>::
      Clone(const T& value) {
    auto lock = boost::lock_guard(m_mutex);
    return Serialization::ShuttleClone(value, *m_sender, *m_receiver);
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename Message, typename Buffer>
  void MessageProtocol<ChannelType, SenderType, EncoderType>::Encode(
      const Message& message, Out<Buffer> buffer) {
    buffer->Append(std::uint32_t{0});
    auto serializationBuffer = Buffer();
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender->SetSink(Ref(serializationBuffer));
      m_sender->Send(message);
    }
    auto encoderViewBuffer = IO::BufferView<typename Channel::Writer::Buffer>(
      Ref(*buffer), sizeof(std::uint32_t));
    auto size = m_encoder->Encode(serializationBuffer,
      Store(encoderViewBuffer));
    buffer->Write(0, ToLittleEndian<std::uint32_t>(size));
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename Message>
  typename std::enable_if<!ImplementsConcept<
      Message, IO::Buffer>::value>::type MessageProtocol<ChannelType,
      SenderType, EncoderType>::Send(const Message& message) {
    auto senderBuffer = typename Channel::Writer::Buffer();
    auto encoderBuffer = typename Channel::Writer::Buffer();
    if(Codecs::InPlaceSupport<Encoder>::value) {
      senderBuffer.Append(std::uint32_t{0});
    } else {
      encoderBuffer.Append(std::uint32_t{0});
    }
    {
      auto lock = boost::lock_guard(m_mutex);
      m_sender->SetSink(Ref(senderBuffer));
      m_sender->Send(message);
    }
    if(Codecs::InPlaceSupport<Encoder>::value) {
      auto senderViewBuffer = IO::BufferView<typename Channel::Writer::Buffer>(
        Ref(senderBuffer), sizeof(std::uint32_t));
      auto size = m_encoder->Encode(senderViewBuffer, Store(senderViewBuffer));
      senderBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
      m_writer.Write(senderBuffer);
    } else {
      auto encoderViewBuffer = IO::BufferView<typename Channel::Writer::Buffer>(
        Ref(encoderBuffer), sizeof(std::uint32_t));
      auto size = m_encoder->Encode(senderBuffer, Store(encoderViewBuffer));
      encoderBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
      m_writer.Write(encoderBuffer);
    }
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename Buffer>
  typename std::enable_if<ImplementsConcept<Buffer, IO::Buffer>::value>::type
      MessageProtocol<ChannelType, SenderType, EncoderType>::Send(
      const Buffer& buffer) {
    m_writer.Write(buffer);
  }

  template<typename ChannelType, typename SenderType, typename EncoderType>
  template<typename Message>
  Message MessageProtocol<ChannelType, SenderType, EncoderType>::Receive() {
    try {
      auto size = std::uint32_t{0};
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
}

#endif
