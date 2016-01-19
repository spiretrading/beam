#include "Beam/CodecsTests/NullEncoderTester.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void NullEncoderTester::TestEmptyEncodeFromBufferToBuffer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEmptyEncodeFromBufferToBufferInPlace() {
  NullEncoder encoder;
  auto buffer = BufferFromString<SharedBuffer>("");
  auto encodeSize = encoder.Encode(buffer, Store(buffer));
  CPPUNIT_ASSERT(encodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == SharedBuffer());
}

void NullEncoderTester::TestEncodeFromBufferToBuffer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEncodeFromBufferToBufferInPlace() {
  NullEncoder encoder;
  auto buffer = BufferFromString<SharedBuffer>("hello");
  auto encodeSize = encoder.Encode(buffer, Store(buffer));
  CPPUNIT_ASSERT(encodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == BufferFromString<SharedBuffer>("hello"));
}

void NullEncoderTester::TestEmptyEncodeFromBufferToPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  char encodedBuffer[16] = {'\0'};
  auto encodeSize = encoder.Encode(message, encodedBuffer,
    sizeof(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEmptyEncodeFromBufferToPointerInPlace() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto encodeSize = encoder.Encode(message, message.GetMutableData(),
    message.GetSize());
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == SharedBuffer());
}

void NullEncoderTester::TestEncodeFromBufferToPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char encodedBuffer[16] = {'\0'};
  auto encodeSize = encoder.Encode(message, encodedBuffer,
    sizeof(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEncodeFromBufferToPointerInPlace() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  auto encodeSize = encoder.Encode(message, message.GetMutableData(),
    message.GetSize());
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == BufferFromString<SharedBuffer>("hello"));
}

void NullEncoderTester::TestEncodeFromBufferToSmallerPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char encodedBuffer[4] = {'\0'};
  CPPUNIT_ASSERT_THROW(encoder.Encode(message, encodedBuffer,
    sizeof(encodedBuffer)), EncoderException);
}

void NullEncoderTester::TestEmptyEncodeFromPointerToBuffer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    Store(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEmptyEncodeFromPointerToBufferInPlace() {
  NullEncoder encoder;
  auto buffer = BufferFromString<SharedBuffer>("");
  auto encodeSize = encoder.Encode(buffer.GetData(), buffer.GetSize(),
    Store(buffer));
  CPPUNIT_ASSERT(encodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == SharedBuffer());
}

void NullEncoderTester::TestEncodeFromPointerToBuffer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    Store(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEncodeFromPointerToBufferInPlace() {
  NullEncoder encoder;
  auto buffer = BufferFromString<SharedBuffer>("hello");
  auto encodeSize = encoder.Encode(buffer.GetData(), buffer.GetSize(),
    Store(buffer));
  CPPUNIT_ASSERT(encodeSize == buffer.GetSize());
  CPPUNIT_ASSERT(buffer == BufferFromString<SharedBuffer>("hello"));
}

void NullEncoderTester::TestEmptyEncodeFromPointerToPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  char encodedBuffer[16] = {'\0'};
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    encodedBuffer, sizeof(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEmptyEncodeFromPointerToPointerInPlace() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    message.GetMutableData(), message.GetSize());
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == SharedBuffer());
}

void NullEncoderTester::TestEncodeFromPointerToPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char encodedBuffer[16] = {'\0'};
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    encodedBuffer, sizeof(encodedBuffer));
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(encodedBuffer == message);
}

void NullEncoderTester::TestEncodeFromPointerToPointerInPlace() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
    message.GetMutableData(), message.GetSize());
  CPPUNIT_ASSERT(encodeSize == message.GetSize());
  CPPUNIT_ASSERT(message == BufferFromString<SharedBuffer>("hello"));
}

void NullEncoderTester::TestEncodeFromPointerToSmallerPointer() {
  NullEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello");
  char encodedBuffer[4] = {'\0'};
  CPPUNIT_ASSERT_THROW(encoder.Encode(message.GetData(), message.GetSize(),
    encodedBuffer, sizeof(encodedBuffer)), EncoderException);
}
