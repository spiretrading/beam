#include "Beam/CodecsTests/NullDecoderTester.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void NullDecoderTester::TestEmptyDecodeFromBufferToBuffer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestEmptyDecodeFromBufferToBufferInPlace() {
  NullDecoder decoder;
  auto buffer = BufferFromString<SharedBuffer>("");
  auto decodeSize = decoder.Decode(buffer, Store(buffer));
  CPPUNIT_ASSERT(decodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == SharedBuffer());
}

void NullDecoderTester::TestDecodeFromBufferToBuffer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestDecodeFromBufferToBufferInPlace() {
  NullDecoder decoder;
  auto buffer = BufferFromString<SharedBuffer>("hello");
  auto decodeSize = decoder.Decode(buffer, Store(buffer));
  CPPUNIT_ASSERT(decodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == BufferFromString<SharedBuffer>("hello"));
}

void NullDecoderTester::TestEmptyDecodeFromBufferToPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  char decodedBuffer[16] = {'\0'};
  auto decodeSize = decoder.Decode(message, decodedBuffer,
    sizeof(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestEmptyDecodeFromBufferToPointerInPlace() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto decodeSize = decoder.Decode(message, message.GetMutableData(),
    message.GetSize());
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == SharedBuffer());
}

void NullDecoderTester::TestDecodeFromBufferToPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char decodedBuffer[16] = {'\0'};
  auto decodeSize = decoder.Decode(message, decodedBuffer,
    sizeof(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestDecodeFromBufferToPointerInPlace() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  auto decodeSize = decoder.Decode(message, message.GetMutableData(),
    message.GetSize());
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == BufferFromString<SharedBuffer>("hello"));
}

void NullDecoderTester::TestDecodeFromBufferToSmallerPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char decodedBuffer[4] = {'\0'};
  CPPUNIT_ASSERT_THROW(decoder.Decode(message, decodedBuffer,
    sizeof(decodedBuffer)), DecoderException);
}

void NullDecoderTester::TestEmptyDecodeFromPointerToBuffer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    Store(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestEmptyDecodeFromPointerToBufferInPlace() {
  NullDecoder decoder;
  auto buffer = BufferFromString<SharedBuffer>("");
  auto decodeSize = decoder.Decode(buffer.GetData(), buffer.GetSize(),
    Store(buffer));
  CPPUNIT_ASSERT(decodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == SharedBuffer());
}

void NullDecoderTester::TestDecodeFromPointerToBuffer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  SharedBuffer decodedBuffer;
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    Store(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestDecodeFromPointerToBufferInPlace() {
  NullDecoder decoder;
  auto buffer = BufferFromString<SharedBuffer>("hello");
  auto decodeSize = decoder.Decode(buffer.GetData(), buffer.GetSize(),
    Store(buffer));
  CPPUNIT_ASSERT(decodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == BufferFromString<SharedBuffer>("hello"));
}

void NullDecoderTester::TestEmptyDecodeFromPointerToPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  char decodedBuffer[16] = {'\0'};
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    decodedBuffer, sizeof(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestEmptyDecodeFromPointerToPointerInPlace() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    message.GetMutableData(), message.GetSize());
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == SharedBuffer());
}

void NullDecoderTester::TestDecodeFromPointerToPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char decodedBuffer[16] = {'\0'};
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    decodedBuffer, sizeof(decodedBuffer));
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void NullDecoderTester::TestDecodeFromPointerToPointerInPlace() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
    message.GetMutableData(), message.GetSize());
  CPPUNIT_ASSERT(decodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == BufferFromString<SharedBuffer>("hello"));
}

void NullDecoderTester::TestDecodeFromPointerToSmallerPointer() {
  NullDecoder decoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char decodedBuffer[4] = {'\0'};
  CPPUNIT_ASSERT_THROW(decoder.Decode(message.GetData(), message.GetSize(),
    decodedBuffer, sizeof(decodedBuffer)), DecoderException);
}
