#include "Beam/IOTests/BufferReaderTester.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::IO::Tests;
using namespace boost;
using namespace std;

void BufferReaderTester::TestCreateEmpty() {
  BufferReader<SharedBuffer> reader(BufferFromString<SharedBuffer>(""));
  SharedBuffer buffer;
  CPPUNIT_ASSERT_THROW(reader.Read(Store(buffer)), EndOfFileException);
}

void BufferReaderTester::TestRead() {
  string message = "hello world";
  BufferReader<SharedBuffer> reader(BufferFromString<SharedBuffer>(message));
  SharedBuffer data;
  size_t sizeRead = reader.Read(Store(data));
  CPPUNIT_ASSERT(message.size() == sizeRead);
  CPPUNIT_ASSERT(data == message);
}

void BufferReaderTester::TestReadSomeToBuffer() {
  string message = "hello world";
  BufferReader<SharedBuffer> reader(BufferFromString<SharedBuffer>(message));
  SharedBuffer data;
  size_t sizeRead = reader.Read(Store(data), 6);
  CPPUNIT_ASSERT(sizeRead == 6);
  CPPUNIT_ASSERT(data == "hello ");
  data.Reset();
  sizeRead = reader.Read(Store(data), 5);
  CPPUNIT_ASSERT(sizeRead == 5);
  CPPUNIT_ASSERT(data == "world");
}

void BufferReaderTester::TestReadSomeToPointer() {
  string message = "hello world";
  BufferReader<SharedBuffer> reader(BufferFromString<SharedBuffer>(message));
  unique_ptr<char[]> data(new char[message.size()]);
  size_t sizeRead = reader.Read(data.get(), 6);
  CPPUNIT_ASSERT(sizeRead == 6);
  CPPUNIT_ASSERT(strncmp(data.get(), "hello ", 6) == 0);
  sizeRead = reader.Read(data.get(), 5);
  CPPUNIT_ASSERT(sizeRead == 5);
  CPPUNIT_ASSERT(strncmp(data.get(), "world", 5) == 0);
}
