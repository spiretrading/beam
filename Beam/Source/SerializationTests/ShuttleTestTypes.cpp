#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

using namespace Beam;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;

ClassWithShuttleMethod::ClassWithShuttleMethod(char a, int b, double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

ClassWithSendReceiveMethods::ClassWithSendReceiveMethods(
  char a, int b, double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

ClassWithVersioning::ClassWithVersioning(int v0, int v1, int v2)
  : m_v0(v0),
    m_v1(v1),
    m_v2(v2) {}

std::string PolymorphicDerivedClassA::ToString() const {
  return "PolymorphicDerivedClassA";
}

std::string PolymorphicDerivedClassB::ToString() const {
  return "PolymorphicDerivedClassB";
}

ProxiedFunctionType::ProxiedFunctionType(const std::string& value)
  : m_value(value) {}

std::string ProxiedFunctionType::ToString() const {
  return m_value;
}

ProxiedMethodType::ProxiedMethodType(const std::string& value)
  : m_value(value) {}

std::string ProxiedMethodType::ToString() const {
  return m_value;
}
