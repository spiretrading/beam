#include "Beam/SerializationTests/ShuttleVariantTester.hpp"
#include <boost/type_traits.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Serialization/ShuttleVariant.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace boost;
using namespace std;

void ShuttleVariantTester::setUp() {
  m_sender.SetSink(Ref(m_buffer));
}

void ShuttleVariantTester::tearDown() {
  m_buffer.Reset();
}

void ShuttleVariantTester::TestSingleType() {
  variant<int> out;
  out = 123;
  m_sender.Shuttle(out);
  variant<int> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}

void ShuttleVariantTester::TestFirstMemberWithTwoTypes() {
  variant<int, string> out;
  out = 123;
  m_sender.Shuttle(out);
  variant<int, string> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}

void ShuttleVariantTester::TestSecondMemberWithTwoTypes() {
  variant<int, string> out;
  out = "hello";
  m_sender.Shuttle(out);
  variant<int, string> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}

void ShuttleVariantTester::TestFirstMemberWithThreeTypes() {
  variant<int, string, double> out;
  out = 123;
  m_sender.Shuttle(out);
  variant<int, string, double> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}

void ShuttleVariantTester::TestSecondMemberWithThreeTypes() {
  variant<int, string, double> out;
  out = "hello world";
  m_sender.Shuttle(out);
  variant<int, string, double> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}

void ShuttleVariantTester::TestThirdMemberWithThreeTypes() {
  variant<int, string, double> out;
  out = 3.1415;
  m_sender.Shuttle(out);
  variant<int, string, double> in;
  m_receiver.SetSource(Ref(m_buffer));
  m_receiver.Shuttle(in);
  CPPUNIT_ASSERT(out == in);
}
