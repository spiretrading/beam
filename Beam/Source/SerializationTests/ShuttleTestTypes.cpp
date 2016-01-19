#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace std;

bool StructWithFreeShuttle::operator ==(const StructWithFreeShuttle& rhs)
    const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithShuttleMethod::ClassWithShuttleMethod() {}

ClassWithShuttleMethod::ClassWithShuttleMethod(char a, int b, double c)
    : m_a(a),
      m_b(b),
      m_c(c) {}

bool ClassWithShuttleMethod::operator ==(
    const ClassWithShuttleMethod& rhs) const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithSendReceiveMethods::ClassWithSendReceiveMethods() {}

ClassWithSendReceiveMethods::ClassWithSendReceiveMethods(char a, int b,
    double c)
    : m_a(a),
      m_b(b),
      m_c(c) {}

bool ClassWithSendReceiveMethods::operator ==(
    const ClassWithSendReceiveMethods& rhs) const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithVersioning::ClassWithVersioning() {}

ClassWithVersioning::ClassWithVersioning(int v0, int v1, int v2)
    : m_v0(v0),
      m_v1(v1),
      m_v2(v2) {}

bool ClassWithVersioning::operator ==(const ClassWithVersioning& rhs) const {
  return m_v0 == rhs.m_v0 && m_v1 == rhs.m_v1 && m_v2 == rhs.m_v2;
}

PolymorphicBaseClass::~PolymorphicBaseClass() {}

PolymorphicDerivedClassA::PolymorphicDerivedClassA() {}

PolymorphicDerivedClassA::~PolymorphicDerivedClassA() {}

string PolymorphicDerivedClassA::ToString() const {
  return "PolymorphicDerivedClassA";
}

PolymorphicDerivedClassB::PolymorphicDerivedClassB() {}

PolymorphicDerivedClassB::~PolymorphicDerivedClassB() {}

string PolymorphicDerivedClassB::ToString() const {
  return "PolymorphicDerivedClassB";
}

ProxiedFunctionType::ProxiedFunctionType() {}

ProxiedFunctionType::ProxiedFunctionType(const string& value)
    : m_value(value) {}

bool ProxiedFunctionType::operator ==(const ProxiedFunctionType& rhs) const {
  return m_value == rhs.m_value;
}

string ProxiedFunctionType::ToString() const {
  return m_value;
}

ProxiedMethodType::ProxiedMethodType() {}

ProxiedMethodType::ProxiedMethodType(const string& value)
    : m_value(value) {}

bool ProxiedMethodType::operator ==(const ProxiedMethodType& rhs) const {
  return m_value == rhs.m_value;
}

string ProxiedMethodType::ToString() const {
  return m_value;
}
