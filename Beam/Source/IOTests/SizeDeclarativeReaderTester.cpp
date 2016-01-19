#include "Beam/IOTests/SizeDeclarativeReaderTester.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::IO::Tests;
using namespace Beam::Routines;
using namespace std;

void SizeDeclarativeReaderTester::TestEmptySource() {
  SizeDeclarativeReader<BufferReader<SharedBuffer>> reader(
    Initialize(BufferFromString<SharedBuffer>("")));
  SharedBuffer buffer;
  CPPUNIT_ASSERT_THROW(reader.Read(Store(buffer)), EndOfFileException);
}

void SizeDeclarativeReaderTester::TestSingleReadMessage() {
  PipedReader<SharedBuffer> pipedReader;
  PipedWriter<SharedBuffer> pipedWriter(Ref(pipedReader));
  SizeDeclarativeReader<PipedReader<SharedBuffer>*> reader(&pipedReader);

  // Build the message.
  string message = "hello world";
  std::uint32_t size = ToLittleEndian(
    static_cast<std::uint32_t>(message.size()));
  pipedWriter.Write(&size, 4);
  pipedWriter.Write(message.c_str(), message.size());

  // Extract the message and test that it is equal to the original message.
  SharedBuffer retrievedMessage;
  CPPUNIT_ASSERT(reader.Read(Store(retrievedMessage)) == message.size());
  CPPUNIT_ASSERT(retrievedMessage.GetSize() == message.size());
  CPPUNIT_ASSERT(string(retrievedMessage.GetData(), message.size()) == message);
}

void SizeDeclarativeReaderTester::TestMultiReadMessage() {
  PipedReader<SharedBuffer> pipedReader;
  PipedWriter<SharedBuffer> pipedWriter(Ref(pipedReader));
  SizeDeclarativeReader<PipedReader<SharedBuffer>*> reader(&pipedReader);

  // Build the message and write the first message fragment.
  string firstFragment = "hello";
  string secondFragment = " world";
  string message = firstFragment + secondFragment;
  std::uint32_t size = ToLittleEndian(static_cast<std::uint32_t>(
    message.size()));
  pipedWriter.Write(&size, 4);
  pipedWriter.Write(firstFragment.c_str(), firstFragment.size());

  // Begin reading the message.
  SharedBuffer retrievedMessage;
  size_t readResult;
  RoutineHandler task = Spawn(
    [&] {
      readResult = reader.Read(Store(retrievedMessage));
    });

  // Write the second message fragment.
  pipedWriter.Write(secondFragment.c_str(), secondFragment.size());
  task.Wait();

  // Extract the message and test that it is equal to the original message.
  CPPUNIT_ASSERT(readResult == message.size());
  CPPUNIT_ASSERT(retrievedMessage.GetSize() == message.size());
  CPPUNIT_ASSERT(string(retrievedMessage.GetData(), message.size()) == message);
}
