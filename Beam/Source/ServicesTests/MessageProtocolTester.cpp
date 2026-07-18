module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/MessageProtocol.hpp"

module Beam;

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
}
