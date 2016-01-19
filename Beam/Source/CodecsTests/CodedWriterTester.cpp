#include "Beam/CodecsTests/CodedWriterTester.hpp"
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;
using namespace std;

void CodedWriterTester::TestSingleByte() {
  PipedReader<SharedBuffer> pipedReader;
  PipedWriter<SharedBuffer> pipedWriter(Ref(pipedReader));
  CodedWriter<PipedWriter<SharedBuffer>*, ReverseEncoder> codedWriter(
    &pipedWriter, ReverseEncoder());
  {
    codedWriter.Write(BufferFromString<SharedBuffer>("a"));
    SharedBuffer readBuffer;
    CPPUNIT_ASSERT(pipedReader.Read(Store(readBuffer)) == 1);
    CPPUNIT_ASSERT(readBuffer.GetSize() == 1);
    CPPUNIT_ASSERT(string(readBuffer.GetData(), readBuffer.GetSize()) == "a");
  }
  {
    codedWriter.Write(BufferFromString<SharedBuffer>("b"));
    SharedBuffer readBuffer;
    CPPUNIT_ASSERT(pipedReader.Read(Store(readBuffer)) == 1);
    CPPUNIT_ASSERT(readBuffer.GetSize() == 1);
    CPPUNIT_ASSERT(string(readBuffer.GetData(), readBuffer.GetSize()) == "b");
  }
}

void CodedWriterTester::TestWrite() {
  PipedReader<SharedBuffer> pipedReader;
  PipedWriter<SharedBuffer> pipedWriter(Ref(pipedReader));
  CodedWriter<PipedWriter<SharedBuffer>*, ReverseEncoder> codedWriter(
    &pipedWriter, ReverseEncoder());
  {
    auto message = string{"hello"};
    auto reversedMessage = string{message.rbegin(), message.rend()};
    codedWriter.Write(BufferFromString<SharedBuffer>(message));
    SharedBuffer readBuffer;
    CPPUNIT_ASSERT(pipedReader.Read(Store(readBuffer)) ==
      reversedMessage.size());
    CPPUNIT_ASSERT(readBuffer.GetSize() == reversedMessage.size());
    CPPUNIT_ASSERT(string(readBuffer.GetData(), readBuffer.GetSize()) ==
      reversedMessage);
  }
  {
    auto message = string{"world"};
    auto reversedMessage = string{message.rbegin(), message.rend()};
    codedWriter.Write(BufferFromString<SharedBuffer>(message));
    SharedBuffer readBuffer;
    CPPUNIT_ASSERT(pipedReader.Read(Store(readBuffer)) ==
      reversedMessage.size());
    CPPUNIT_ASSERT(readBuffer.GetSize() == reversedMessage.size());
    CPPUNIT_ASSERT(string(readBuffer.GetData(), readBuffer.GetSize()) ==
      reversedMessage);
  }
}
