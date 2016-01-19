#include "Beam/QueriesTests/NativeDataTypeTester.hpp"
#include <string>
#include "Beam/Queries/NativeDataType.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void NativeDataTypeTester::TestInt() {
  NativeDataType<int> nativeType;
  CPPUNIT_ASSERT(nativeType.GetNativeType() == typeid(int));
  DataType type(nativeType);
  CPPUNIT_ASSERT(type->GetNativeType() == typeid(int));
  CPPUNIT_ASSERT(type == nativeType);
  CPPUNIT_ASSERT(type != NativeDataType<string>());
}

void NativeDataTypeTester::TestString() {
  NativeDataType<string> nativeType;
  CPPUNIT_ASSERT(nativeType.GetNativeType() == typeid(string));
  DataType type(nativeType);
  CPPUNIT_ASSERT(type->GetNativeType() == typeid(string));
  CPPUNIT_ASSERT(type == nativeType);
  CPPUNIT_ASSERT(type != NativeDataType<int>());
}
