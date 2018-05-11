#include "Beam/CodecsTests/ZLibCodecTester.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void ZLibCodecTester::TestEmptyMessage() {
  ZLibEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
  ZLibDecoder decoder;
  SharedBuffer decodedBuffer;
  auto decodedSize = decoder.Decode(encodedBuffer, Store(decodedBuffer));
  CPPUNIT_ASSERT(decodedBuffer == message);
}

void ZLibCodecTester::TestSimpleMessage() {
  ZLibEncoder encoder;
  auto message = BufferFromString<SharedBuffer>("hello world");
  SharedBuffer encodedBuffer;
  auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
  ZLibDecoder decoder;
  SharedBuffer decodedBuffer;
  auto decodedSize = decoder.Decode(encodedBuffer, Store(decodedBuffer));
  CPPUNIT_ASSERT(decodedBuffer == message);
}
