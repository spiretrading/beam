#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

using namespace Beam;
using namespace Beam::Tests;

std::ostream& Beam::Tests::operator <<(
    std::ostream& out, const StructWithFreeShuttle& value) {
  return out << '(' << value.m_a << ", " << value.m_b << ", " << value.m_c <<
    ')';
}

ClassWithShuttleMethod::ClassWithShuttleMethod(char a, int b, double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

std::ostream& Beam::Tests::operator <<(
    std::ostream& out, const ClassWithShuttleMethod& value) {
  return out << '(' << value.m_a << ", " << value.m_b << ", " << value.m_c <<
    ')';
}

ClassWithSendReceiveMethods::ClassWithSendReceiveMethods(
  char a, int b, double c)
  : m_a(a),
    m_b(b),
    m_c(c) {}

std::ostream& Beam::Tests::operator <<(
    std::ostream& out, const ClassWithSendReceiveMethods& value) {
  return out << '(' << value.m_a << ", " << value.m_b << ", " << value.m_c <<
    ')';
}

ClassWithVersioning::ClassWithVersioning(int v0, int v1, int v2)
  : m_v0(v0),
    m_v1(v1),
    m_v2(v2) {}

std::ostream& Beam::Tests::operator <<(
    std::ostream& out, const ClassWithVersioning& value) {
  return out << '(' << value.m_v0 << ", " << value.m_v1 << ", " << value.m_v2 <<
    ')';
}

std::string PolymorphicDerivedClassA::to_string() const {
  return "PolymorphicDerivedClassA";
}

std::string PolymorphicDerivedClassB::to_string() const {
  return "PolymorphicDerivedClassB";
}

ProxiedFunctionType::ProxiedFunctionType(const std::string& value)
  : m_value(value) {}

std::string ProxiedFunctionType::to_string() const {
  return m_value;
}

ProxiedMethodType::ProxiedMethodType(const std::string& value)
  : m_value(value) {}

std::string ProxiedMethodType::to_string() const {
  return m_value;
}
