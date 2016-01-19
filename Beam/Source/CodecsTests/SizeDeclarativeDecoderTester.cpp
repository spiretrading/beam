#include "Beam/CodecsTests/SizeDeclarativeDecoderTester.hpp"
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void SizeDeclarativeDecoderTester::TestEmptyDecodeFromBufferToBuffer() {
  SizeDeclarativeDecoder<ReverseDecoder> decoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto decodedMessage = BufferFromString<SharedBuffer>("");
  SharedBuffer outputBuffer;
  outputBuffer.Append(ToBigEndian<uint32_t>(message.GetSize()));
  outputBuffer.Append(message);
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(outputBuffer, Store(decodedBuffer));
  auto expectedDecodeSize = decodedMessage.GetSize();
  CPPUNIT_ASSERT(decodeSize == expectedDecodeSize);
  CPPUNIT_ASSERT(decodedBuffer == decodedMessage);
}

void SizeDeclarativeDecoderTester::TestDecodeFromBufferToBuffer() {
  SizeDeclarativeDecoder<ReverseDecoder> decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  auto decodedMessage = BufferFromString<SharedBuffer>("olleh");
  SharedBuffer outputBuffer;
  outputBuffer.Append(ToBigEndian<uint32_t>(message.GetSize()));
  outputBuffer.Append(message);
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(outputBuffer, Store(decodedBuffer));
  auto expectedDecodeSize = decodedMessage.GetSize();
  CPPUNIT_ASSERT(decodeSize == expectedDecodeSize);
  CPPUNIT_ASSERT(decodedBuffer == decodedMessage);
}

void SizeDeclarativeDecoderTester::TestEmptyDecodeFromBufferToPointer() {
  SizeDeclarativeDecoder<ReverseDecoder> decoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto decodedMessage = BufferFromString<SharedBuffer>("");
  SharedBuffer outputBuffer;
  outputBuffer.Append(ToBigEndian<uint32_t>(message.GetSize()));
  outputBuffer.Append(message);
  SharedBuffer decodedBuffer;
  decodedBuffer.Reserve(decodedMessage.GetSize());
  auto decodeSize = decoder.Decode(outputBuffer,
    decodedMessage.GetMutableData(), decodedMessage.GetSize());
  auto expectedDecodeSize = decodedMessage.GetSize();
  CPPUNIT_ASSERT(decodeSize == expectedDecodeSize);
  CPPUNIT_ASSERT(decodedBuffer == decodedMessage);
}
