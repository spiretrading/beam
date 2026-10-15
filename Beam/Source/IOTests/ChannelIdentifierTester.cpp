#include <doctest/doctest.h>
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  struct SessionIdentifier {
    int m_id;
  };

  std::ostream& operator <<(
      std::ostream& out, const SessionIdentifier& identifier) {
    return out << "session:" << identifier.m_id;
  }
}

TEST_SUITE("ChannelIdentifier") {
  TEST_CASE("format") {
    auto identifier = ChannelIdentifier(NamedChannelIdentifier("feed"));
    REQUIRE(std::format("{}", identifier) == "feed");
    REQUIRE(std::format("{:*^8}", identifier) == "**feed**");
    REQUIRE(std::format("{:.3}", identifier) == "fee");
    REQUIRE(to_string(identifier) == "feed");
    REQUIRE(
      std::format("{}", ChannelIdentifier(NamedChannelIdentifier())).empty());
    auto session = ChannelIdentifier(SessionIdentifier(17));
    REQUIRE(std::format("{}", session) == "session:17");
    REQUIRE(std::format("{:*>12}", session) == "**session:17");
    REQUIRE(std::format("{:.7}", session) == "session");
    auto& base = static_cast<const Streamable&>(session);
    REQUIRE(std::format("{}", base) == "session:17");
    REQUIRE(to_string(session) == "session:17");
  }
}
