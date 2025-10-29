#include <sstream>
#include <doctest/doctest.h>
#include "Beam/WebServices/EmailAddress.hpp"

using namespace Beam;

TEST_SUITE("EmailAddress") {
  TEST_CASE("construct_address_only") {
    auto email = EmailAddress("user@example.com");
    REQUIRE(email.get_address() == "user@example.com");
    REQUIRE(email.get_display_name().empty());
    REQUIRE(email.get_user() == "user");
    REQUIRE(email.get_domain() == "example.com");
  }

  TEST_CASE("construct_with_display_name") {
    auto email = EmailAddress("admin@test.com", "Administrator");
    REQUIRE(email.get_address() == "admin@test.com");
    REQUIRE(email.get_display_name() == "Administrator");
    REQUIRE(email.get_user() == "admin");
    REQUIRE(email.get_domain() == "test.com");
  }

  TEST_CASE("get_user_and_domain") {
    auto email = EmailAddress("john.doe@company.org");
    REQUIRE(email.get_user() == "john.doe");
    REQUIRE(email.get_domain() == "company.org");
  }

  TEST_CASE("get_user_without_at_sign") {
    auto email = EmailAddress("invalidaddress");
    REQUIRE(email.get_user() == "invalidaddress");
    REQUIRE(email.get_domain().empty());
  }

  TEST_CASE("get_domain_with_subdomain") {
    auto email = EmailAddress("support@mail.example.com");
    REQUIRE(email.get_user() == "support");
    REQUIRE(email.get_domain() == "mail.example.com");
  }

  TEST_CASE("stream_without_display_name") {
    auto email = EmailAddress("test@example.com");
    auto ss = std::stringstream();
    ss << email;
    REQUIRE(ss.str() == "test@example.com");
  }

  TEST_CASE("stream_with_display_name") {
    auto email = EmailAddress("contact@company.com", "Contact Us");
    auto ss = std::stringstream();
    ss << email;
    REQUIRE(ss.str() == "Contact Us <contact@company.com>");
  }

  TEST_CASE("empty_address") {
    auto email = EmailAddress("");
    REQUIRE(email.get_address().empty());
    REQUIRE(email.get_display_name().empty());
    REQUIRE(email.get_user().empty());
    REQUIRE(email.get_domain().empty());
  }

  TEST_CASE("complex_user_portion") {
    auto email = EmailAddress("first.last+tag@example.com");
    REQUIRE(email.get_user() == "first.last+tag");
    REQUIRE(email.get_domain() == "example.com");
  }

  TEST_CASE("multiple_at_signs") {
    auto email = EmailAddress("user@domain@invalid.com");
    REQUIRE(email.get_user() == "user");
    REQUIRE(email.get_domain() == "domain@invalid.com");
  }
}
