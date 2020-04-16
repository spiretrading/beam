#include <string>
#include <doctest/doctest.h>
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/NullWriter.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/MessageProtocol.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Services;

TEST_SUITE("MessageProtocol") {
  TEST_CASE("send_message") {
    using ProtocolChannel = BasicChannel<NamedChannelIdentifier, NullConnection,
      NullReader, PipedWriter<SharedBuffer>>;
    auto reader = PipedReader<SharedBuffer>();
    auto channel = ProtocolChannel("channel", Initialize(), Initialize(),
      Initialize(Ref(reader)));
    auto protocol = MessageProtocol<ProtocolChannel*,
      BinarySender<SharedBuffer>, ReverseEncoder>(&channel,
      BinarySender<SharedBuffer>(), BinaryReceiver<SharedBuffer>(),
      ReverseEncoder(), ReverseDecoder());
    protocol.Send(std::string("hello world"));
    auto sourceBuffer = SharedBuffer();
    auto targetBuffer = SharedBuffer();
    reader.Read(Store(sourceBuffer));
    auto decoder = ReverseDecoder();
    decoder.Decode(sourceBuffer, Store(targetBuffer));
    auto receiver = BinaryReceiver<SharedBuffer>();
    receiver.SetSource(Ref(targetBuffer));
    auto message = std::string();
    receiver.Shuttle(message);
    REQUIRE(message == "hello world");
  }

  TEST_CASE("receive_message") {
    using ProtocolChannel = BasicChannel<NamedChannelIdentifier, NullConnection,
      PipedReader<SharedBuffer>*, NullWriter>;
    auto reader = PipedReader<SharedBuffer>();
    auto writer = PipedWriter<SharedBuffer>(Ref(reader));
    auto channel = ProtocolChannel("channel", Initialize(), &reader,
      Initialize());
    auto protocol = MessageProtocol<ProtocolChannel*,
      BinarySender<SharedBuffer>, ReverseEncoder>(&channel,
      BinarySender<SharedBuffer>(), BinaryReceiver<SharedBuffer>(),
      ReverseEncoder(), ReverseDecoder());
    auto sentMessage = std::string("hello world");
    auto sender = BinarySender<SharedBuffer>();
    auto encodingBuffer = SharedBuffer();
    sender.SetSink(Ref(encodingBuffer));
    sender.Send(sentMessage);
    auto encoder = ReverseEncoder();
    auto destinationBuffer = SharedBuffer();
    destinationBuffer.Append(std::uint32_t{0});
    auto size = encoder.Encode(encodingBuffer, Store(destinationBuffer));
    destinationBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
    writer.Write(destinationBuffer);
    auto receivedMessage = protocol.Receive<std::string>();
    REQUIRE(receivedMessage == sentMessage);
  }
}
