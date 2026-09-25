#include <string>
#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/MessageProtocol.hpp"
#include "Beam/Utilities/ReportException.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::endian;

TEST_SUITE("MessageProtocol") {
  TEST_CASE("message") {
    using ProtocolChannel = BasicChannel<
      NamedChannelIdentifier, NullConnection, PipedReader*, PipedWriter*>;
    auto receive_reader = PipedReader();
    auto send_writer = PipedWriter(Ref(receive_reader));
    auto send_reader = PipedReader();
    auto receive_writer = PipedWriter(Ref(send_reader));
    auto send_channel =
      ProtocolChannel("sender", init(), &send_reader, &send_writer);
    auto sender = MessageProtocol(&send_channel, BinarySender<SharedBuffer>(),
      BinaryReceiver<SharedBuffer>(), ReverseEncoder(), ReverseDecoder());
    auto receive_channel =
      ProtocolChannel("receiver", init(), &receive_reader, &receive_writer);
    auto receiver = MessageProtocol(&receive_channel,
      BinarySender<SharedBuffer>(), BinaryReceiver<SharedBuffer>(),
      ReverseEncoder(), ReverseDecoder());
    auto sent_message = std::string("hello world");
    sender.send(sent_message);
    auto received_message = receiver.receive<std::string>();
    REQUIRE(received_message == sent_message);
  }

  TEST_CASE("in_place_message") {
    using ProtocolChannel = BasicChannel<
      NamedChannelIdentifier, NullConnection, PipedReader*, PipedWriter*>;
    auto receive_reader = PipedReader();
    auto send_writer = PipedWriter(Ref(receive_reader));
    auto send_reader = PipedReader();
    auto receive_writer = PipedWriter(Ref(send_reader));
    auto send_channel =
      ProtocolChannel("sender", init(), &send_reader, &send_writer);
    auto sender = MessageProtocol(&send_channel, BinarySender<SharedBuffer>(),
      BinaryReceiver<SharedBuffer>(), NullEncoder(), NullDecoder());
    auto receive_channel =
      ProtocolChannel("receiver", init(), &receive_reader, &receive_writer);
    auto receiver = MessageProtocol(&receive_channel,
      BinarySender<SharedBuffer>(), BinaryReceiver<SharedBuffer>(),
      NullEncoder(), NullDecoder());
    auto sent_message = std::string("hello world");
    sender.send(sent_message);
    auto received_message = receiver.receive<std::string>();
    REQUIRE(received_message == sent_message);
  }

  TEST_CASE("decoder_diagnostics") {
    using ProtocolChannel = BasicChannel<
      NamedChannelIdentifier, NullConnection, PipedReader*, PipedWriter*>;
    auto receive_reader = PipedReader();
    auto send_writer = PipedWriter(Ref(receive_reader));
    auto send_reader = PipedReader();
    auto receive_writer = PipedWriter(Ref(send_reader));
    auto send_channel =
      ProtocolChannel("sender", init(), &send_reader, &send_writer);
    auto sender = MessageProtocol(&send_channel, BinarySender<SharedBuffer>(),
      BinaryReceiver<SharedBuffer>(), SizeDeclarativeEncoder<ReverseEncoder>(),
      SizeDeclarativeDecoder<ReverseDecoder>());
    auto receive_channel =
      ProtocolChannel("receiver", init(), &receive_reader, &receive_writer);
    auto receiver = MessageProtocol(&receive_channel,
      BinarySender<SharedBuffer>(), BinaryReceiver<SharedBuffer>(),
      SizeDeclarativeEncoder<ReverseEncoder>(),
      SizeDeclarativeDecoder<ReverseDecoder>());
    auto invalid = SharedBuffer();
    append(invalid, native_to_little<std::uint32_t>(9));
    append(invalid, native_to_big<std::uint32_t>(100));
    append(invalid, "hello", 5);
    send_writer.write(invalid);
    auto report = [&] {
      try {
        receiver.receive<std::string>();
      } catch(const DecoderException&) {
        return make_exception_report();
      }
      return std::string();
    }();
    REQUIRE(report.contains("channel=receiver"));
    REQUIRE(report.contains("encoded_size=9"));
    REQUIRE(report.contains("declared_size=100"));
    REQUIRE(report.contains("decoded_size=5"));
    auto message = std::string("hello world");
    sender.send(message);
    REQUIRE(receiver.receive<std::string>() == message);
  }
}
