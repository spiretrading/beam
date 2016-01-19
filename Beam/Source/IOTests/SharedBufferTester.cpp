#include "Beam/IOTests/SharedBufferTester.hpp"
#include <cstring>
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::IO::Tests;
using namespace std;

void SharedBufferTester::TestCreateEmpty() {
  SharedBuffer buffer;
  CPPUNIT_ASSERT(buffer.GetSize() == 0);
  CPPUNIT_ASSERT(buffer.GetData() == nullptr);
}

void SharedBufferTester::TestCreateInitialSize() {
  SharedBuffer buffer(1);
  CPPUNIT_ASSERT(buffer.GetSize() == 1);
  CPPUNIT_ASSERT(buffer.GetData() != nullptr);
}

void SharedBufferTester::TestGrowingEmptyBuffer() {
  SharedBuffer buffer;
  int size = 0;
  buffer.Grow(1);
  size += 1;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(2);
  size += 2;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(15);
  size += 15;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(1);
  size += 1;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(3141);
  size += 3141;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
}

void SharedBufferTester::TestGrowingInitialSizedBuffer() {
  SharedBuffer buffer(1);
  int size = 1;
  buffer.Grow(1);
  size += 1;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(2);
  size += 2;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(15);
  size += 15;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(1);
  size += 1;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
  buffer.Grow(3141);
  size += 3141;
  CPPUNIT_ASSERT(buffer.GetSize() == size);
}

void SharedBufferTester::TestCopy() {
  const char* message = "hello world";
  int messageSize = static_cast<int>(strlen(message));
  SharedBuffer buffer(messageSize);
  strncpy(buffer.GetMutableData(), "hello world", messageSize);
  SharedBuffer copy = buffer;
  CPPUNIT_ASSERT(buffer.GetSize() == copy.GetSize());

  // Test copy on write semantics.
  CPPUNIT_ASSERT(buffer.GetData() == copy.GetData());
  copy.Append("a", 1);
  CPPUNIT_ASSERT(buffer.GetData() != copy.GetData());
}

void SharedBufferTester::TestAppend() {
  SharedBuffer buffer;
  const char* message = "hello world";
  const char* leftMessage = "hello";
  const char* rightMessage = " world";
  int leftMessageSize = static_cast<int>(strlen(leftMessage));
  int rightMessageSize = static_cast<int>(strlen(rightMessage));

  buffer.Append(leftMessage, leftMessageSize);
  CPPUNIT_ASSERT(buffer.GetSize() == leftMessageSize);
  CPPUNIT_ASSERT(strncmp(buffer.GetData(), leftMessage, leftMessageSize) == 0);

  buffer.Append(rightMessage, rightMessageSize);
  CPPUNIT_ASSERT(buffer.GetSize() == leftMessageSize + rightMessageSize);
  CPPUNIT_ASSERT(strncmp(buffer.GetData(), message,
    leftMessageSize + rightMessageSize) == 0);
}

void SharedBufferTester::TestReset() {
  SharedBuffer buffer;
  buffer.Append("a", 1);
  buffer.Reset();
  CPPUNIT_ASSERT(buffer.GetSize() == 0);
}

void SharedBufferTester::TestCopyOnWriteWithAppendToOriginal() {
  SharedBuffer buffer;
  buffer.Append("a", 1);
  SharedBuffer copy = buffer;
  buffer.Append("b", 1);
  CPPUNIT_ASSERT(buffer.GetData() != copy.GetData());  
}

void SharedBufferTester::TestCopyOnWriteWithAppendToCopy() {
  SharedBuffer buffer;
  buffer.Append("a", 1);
  SharedBuffer copy = buffer;
  copy.Append("b", 1);
  CPPUNIT_ASSERT(buffer.GetData() != copy.GetData());  
}
