#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;

bool StructWithFreeShuttle::operator ==(const StructWithFreeShuttle& rhs)
    const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithShuttleMethod::ClassWithShuttleMethod(char a, int b, double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

bool ClassWithShuttleMethod::operator ==(
    const ClassWithShuttleMethod& rhs) const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithSendReceiveMethods::ClassWithSendReceiveMethods(char a, int b,
  double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

bool ClassWithSendReceiveMethods::operator ==(
    const ClassWithSendReceiveMethods& rhs) const {
  return m_a == rhs.m_a && m_b == rhs.m_b && m_c == rhs.m_c;
}

ClassWithVersioning::ClassWithVersioning(int v0, int v1, int v2)
  : m_v0(v0),
    m_v1(v1),
    m_v2(v2) {}

bool ClassWithVersioning::operator ==(const ClassWithVersioning& rhs) const {
  return m_v0 == rhs.m_v0 && m_v1 == rhs.m_v1 && m_v2 == rhs.m_v2;
}

std::string PolymorphicDerivedClassA::ToString() const {
  return "PolymorphicDerivedClassA";
}

std::string PolymorphicDerivedClassB::ToString() const {
  return "PolymorphicDerivedClassB";
}

ProxiedFunctionType::ProxiedFunctionType(const std::string& value)
  : m_value(value) {}

bool ProxiedFunctionType::operator ==(const ProxiedFunctionType& rhs) const {
  return m_value == rhs.m_value;
}

std::string ProxiedFunctionType::ToString() const {
  return m_value;
}

ProxiedMethodType::ProxiedMethodType(const std::string& value)
  : m_value(value) {}

bool ProxiedMethodType::operator ==(const ProxiedMethodType& rhs) const {
  return m_value == rhs.m_value;
}

std::string ProxiedMethodType::ToString() const {
  return m_value;
}
