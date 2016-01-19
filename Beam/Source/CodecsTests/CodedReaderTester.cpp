#include "Beam/CodecsTests/CodedReaderTester.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void CodedReaderTester::TestEmpty() {
  CodedReader<SharedBuffer, BufferReader<SharedBuffer>, ReverseDecoder>
    codedReader(Initialize(BufferFromString<SharedBuffer>("")),
    ReverseDecoder());
  SharedBuffer buffer;
  CPPUNIT_ASSERT_THROW(codedReader.Read(Store(buffer)), EndOfFileException);
}

void CodedReaderTester::TestSingleByte() {
  CodedReader<SharedBuffer, BufferReader<SharedBuffer>, ReverseDecoder>
    codedReader(Initialize(BufferFromString<SharedBuffer>("a")),
    ReverseDecoder());
  SharedBuffer buffer;
  CPPUNIT_ASSERT(codedReader.Read(Store(buffer)) == 1);
  CPPUNIT_ASSERT(buffer.GetSize() == 1);
  CPPUNIT_ASSERT(buffer.GetData()[0] == 'a');
  CPPUNIT_ASSERT_THROW(codedReader.Read(Store(buffer)), EndOfFileException);
}

void CodedReaderTester::TestRead() {
  string message = "hello world";
  string reverse = "dlrow olleh";
  CodedReader<SharedBuffer, BufferReader<SharedBuffer>, ReverseDecoder>
    codedReader(Initialize(BufferFromString<SharedBuffer>(message)),
    ReverseDecoder());
  SharedBuffer buffer;
  CPPUNIT_ASSERT(codedReader.Read(Store(buffer)) ==
    static_cast<int>(reverse.size()));
  CPPUNIT_ASSERT(buffer.GetSize() == static_cast<int>(reverse.size()));
  CPPUNIT_ASSERT(string(buffer.GetData(), buffer.GetSize()) == reverse);
  CPPUNIT_ASSERT_THROW(codedReader.Read(Store(buffer)), EndOfFileException);
}

void CodedReaderTester::TestReadSome() {
  string message = "helloworld";
  string firstReverse = "dlrow";
  string secondReverse = "olleh";
  CodedReader<SharedBuffer, BufferReader<SharedBuffer>, ReverseDecoder>
    codedReader(Initialize(BufferFromString<SharedBuffer>(message)),
    ReverseDecoder());
  SharedBuffer buffer;
  CPPUNIT_ASSERT(codedReader.Read(Store(buffer),
    static_cast<int>(firstReverse.size())) ==
    static_cast<int>(firstReverse.size()));
  CPPUNIT_ASSERT(buffer.GetSize() == static_cast<int>(firstReverse.size()));
  CPPUNIT_ASSERT(string(buffer.GetData(), buffer.GetSize()) == firstReverse);
  buffer.Reset();
  CPPUNIT_ASSERT(codedReader.Read(Store(buffer),
    static_cast<int>(secondReverse.size())) ==
    static_cast<int>(secondReverse.size()));
  CPPUNIT_ASSERT(buffer.GetSize() == static_cast<int>(secondReverse.size()));
  CPPUNIT_ASSERT(string(buffer.GetData(), buffer.GetSize()) == secondReverse);
  CPPUNIT_ASSERT_THROW(codedReader.Read(Store(buffer)), EndOfFileException);
}
