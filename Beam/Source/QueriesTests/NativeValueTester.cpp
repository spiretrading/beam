#include "Beam/QueriesTests/NativeValueTester.hpp"
#include <string>
#include "Beam/Queries/NativeValue.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void NativeValueTester::TestMakeNativeValue() {
  auto intValue = MakeNativeValue(5);
  CPPUNIT_ASSERT(intValue.GetType()->GetNativeType() == typeid(int));
  auto stringValue = MakeNativeValue(string("hello world"));
  CPPUNIT_ASSERT(stringValue.GetType()->GetNativeType() == typeid(string));
}

void NativeValueTester::TestInt() {
  NativeValue<NativeDataType<int>> nativeValue(123);
  CPPUNIT_ASSERT(nativeValue.GetType() == NativeDataType<int>());
  CPPUNIT_ASSERT(nativeValue.GetValue<int>() == 123);
  NativeValue<NativeDataType<int>> defaultValue;
  CPPUNIT_ASSERT(defaultValue.GetType() == NativeDataType<int>());
  CPPUNIT_ASSERT(defaultValue.GetValue<int>() == 0);
}

void NativeValueTester::TestDecimal() {
  NativeValue<NativeDataType<double>> nativeValue(3.14);
  CPPUNIT_ASSERT(nativeValue.GetType() == NativeDataType<double>());
  CPPUNIT_ASSERT(nativeValue.GetValue<double>() == 3.14);
  NativeValue<NativeDataType<double>> defaultValue;
  CPPUNIT_ASSERT(defaultValue.GetType() == NativeDataType<double>());
  CPPUNIT_ASSERT(defaultValue.GetValue<double>() == 0);
}

void NativeValueTester::TestString() {
  NativeValue<NativeDataType<string>> nativeValue("hello world");
  CPPUNIT_ASSERT(nativeValue.GetType() == NativeDataType<string>());
  CPPUNIT_ASSERT(nativeValue.GetValue<string>() == "hello world");
  NativeValue<NativeDataType<string>> defaultValue;
  CPPUNIT_ASSERT(defaultValue.GetType() == NativeDataType<string>());
  CPPUNIT_ASSERT(defaultValue.GetValue<string>().empty());
}
