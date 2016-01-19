#include "Beam/ServicesTests/MessageProtocolTester.hpp"
#include <string>
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
using namespace Beam::Services::Tests;
using namespace std;

void MessageProtocolTester::TestSendMessage() {
  using ProtocolChannel = BasicChannel<NamedChannelIdentifier, NullConnection,
    NullReader, PipedWriter<SharedBuffer>>;
  PipedReader<SharedBuffer> reader;
  ProtocolChannel channel("channel", Initialize(), Initialize(),
    Initialize(Ref(reader)));
  MessageProtocol<ProtocolChannel*, BinarySender<SharedBuffer>, ReverseEncoder>
    protocol(&channel, BinarySender<SharedBuffer>(),
    BinaryReceiver<SharedBuffer>(), ReverseEncoder(), ReverseDecoder());
  protocol.Send(string{"hello world"});
  SharedBuffer sourceBuffer;
  SharedBuffer targetBuffer;
  reader.Read(Store(sourceBuffer));
  ReverseDecoder decoder;
  decoder.Decode(sourceBuffer, Store(targetBuffer));
  BinaryReceiver<SharedBuffer> receiver;
  receiver.SetSource(Ref(targetBuffer));
  string message;
  receiver.Shuttle(message);
  CPPUNIT_ASSERT(message == "hello world");
}

void MessageProtocolTester::TestReceiveMessage() {
  using ProtocolChannel = BasicChannel<NamedChannelIdentifier, NullConnection,
    PipedReader<SharedBuffer>*, NullWriter>;
  PipedReader<SharedBuffer> reader;
  PipedWriter<SharedBuffer> writer(Ref(reader));
  ProtocolChannel channel("channel", Initialize(), &reader, Initialize());
  MessageProtocol<ProtocolChannel*, BinarySender<SharedBuffer>, ReverseEncoder>
    protocol(&channel, BinarySender<SharedBuffer>(),
    BinaryReceiver<SharedBuffer>(), ReverseEncoder(), ReverseDecoder());
  string sentMessage = "hello world";
  BinarySender<SharedBuffer> sender;
  SharedBuffer encodingBuffer;
  sender.SetSink(Ref(encodingBuffer));
  sender.Send(sentMessage);
  ReverseEncoder encoder;
  SharedBuffer destinationBuffer;
  destinationBuffer.Append(std::uint32_t{0});
  auto size = encoder.Encode(encodingBuffer, Store(destinationBuffer));
  destinationBuffer.Write(0, ToLittleEndian<std::uint32_t>(size));
  writer.Write(destinationBuffer);
  auto receivedMessage = protocol.Receive<string>();
  CPPUNIT_ASSERT(receivedMessage == sentMessage);
}
