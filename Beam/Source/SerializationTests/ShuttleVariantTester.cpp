#include <doctest/doctest.h>
#include <boost/variant/variant.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/ShuttleVariant.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;

namespace {
  struct Fixture {
    BinaryReceiver<SharedBuffer> m_receiver;
    BinarySender<SharedBuffer> m_sender;
    SharedBuffer m_buffer;

    Fixture() {
      m_sender.SetSink(Ref(m_buffer));
    }
  };
}

TEST_SUITE("ShuttleVariant") {
  TEST_CASE_FIXTURE(Fixture, "single_type") {
    auto out = variant<int>();
    out = 123;
    m_sender.Shuttle(out);
    auto in = variant<int>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }

  TEST_CASE_FIXTURE(Fixture, "first_member_with_two_types") {
    auto out = variant<int, std::string>();
    out = 123;
    m_sender.Shuttle(out);
    auto in = variant<int, std::string>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }

  TEST_CASE_FIXTURE(Fixture, "second_member_with_two_types") {
    auto out = variant<int, std::string>();
    out = "hello";
    m_sender.Shuttle(out);
    auto in = variant<int, std::string>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }

  TEST_CASE_FIXTURE(Fixture, "first_member_with_three_types") {
    auto out = variant<int, std::string, double>();
    out = 123;
    m_sender.Shuttle(out);
    auto in = variant<int, std::string, double>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }

  TEST_CASE_FIXTURE(Fixture, "second_member_with_three_types") {
    auto out = variant<int, std::string, double>();
    out = "hello world";
    m_sender.Shuttle(out);
    auto in = variant<int, std::string, double>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }

  TEST_CASE_FIXTURE(Fixture, "third_member_with_three_types") {
    auto out = variant<int, std::string, double>();
    out = 3.1415;
    m_sender.Shuttle(out);
    auto in = variant<int, std::string, double>();
    m_receiver.SetSource(Ref(m_buffer));
    m_receiver.Shuttle(in);
    REQUIRE(out == in);
  }
}
