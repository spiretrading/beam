#include <sstream>
#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/Uri.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace boost;

TEST_SUITE("Uri") {
  TEST_CASE("scheme_only") {
    auto uri = Uri("tcp:");
    REQUIRE(uri.GetScheme() == "tcp");
    REQUIRE(uri.GetUsername().empty());
    REQUIRE(uri.GetPassword().empty());
    REQUIRE(uri.GetHostname().empty());
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "tcp:");
  }

  TEST_CASE("hostname_only") {
    auto uri = Uri("//localhost");
    REQUIRE(uri.GetScheme().empty());
    REQUIRE(uri.GetUsername().empty());
    REQUIRE(uri.GetPassword().empty());
    REQUIRE(uri.GetHostname() == "localhost");
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "//localhost");
  }

  TEST_CASE("scheme_and_empty_hostname") {
    auto uri = Uri("tcp:/");
    REQUIRE(uri.GetScheme() == "tcp");
    REQUIRE(uri.GetUsername().empty());
    REQUIRE(uri.GetPassword().empty());
    REQUIRE(uri.GetHostname().empty());
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath() == "/");
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "tcp:/");
  }

  TEST_CASE("scheme_and_hostname") {
    auto uri = Uri("tcp://localhost");
    REQUIRE(uri.GetScheme() == "tcp");
    REQUIRE(uri.GetUsername().empty());
    REQUIRE(uri.GetPassword().empty());
    REQUIRE(uri.GetHostname() == "localhost");
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "tcp://localhost");
  }

  TEST_CASE("username_only") {
    auto uri = Uri("//a:@localhost");
    REQUIRE(uri.GetScheme().empty());
    REQUIRE(uri.GetUsername() == "a");
    REQUIRE(uri.GetPassword().empty());
    REQUIRE(uri.GetHostname() == "localhost");
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "//a:@localhost");
  }

  TEST_CASE("password_only") {
    auto uri = Uri("//:b@localhost");
    REQUIRE(uri.GetScheme().empty());
    REQUIRE(uri.GetUsername().empty());
    REQUIRE(uri.GetPassword() == "b");
    REQUIRE(uri.GetHostname() == "localhost");
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "//:b@localhost");
  }

  TEST_CASE("username_and_password") {
    auto uri = Uri("//a:b@localhost");
    REQUIRE(uri.GetScheme().empty());
    REQUIRE(uri.GetUsername() == "a");
    REQUIRE(uri.GetPassword() == "b");
    REQUIRE(uri.GetHostname() == "localhost");
    REQUIRE(uri.GetPort() == 0);
    REQUIRE(uri.GetPath().empty());
    REQUIRE(uri.GetQuery().empty());
    REQUIRE(uri.GetFragment().empty());
    REQUIRE(lexical_cast<std::string>(uri) == "//a:b@localhost");
  }
}
