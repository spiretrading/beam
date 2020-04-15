#include <doctest/doctest.h>
#include "Beam/WebServices/HttpRequestParser.hpp"

using namespace Beam;
using namespace Beam::WebServices;

TEST_SUITE("HttpRequestParser") {
  TEST_CASE("valid_request") {
    auto parser = HttpRequestParser();
    auto requestString =
      "GET /path/file.html HTTP/1.0\r\n"
      "From: someuser@jmarshall.com\r\n"
      "User-Agent: HTTPTool/1.0\r\n"
      "Cookie: theme=light; sessionToken=abc123\r\n"
      "\r\n";
    parser.Feed(requestString, std::strlen(requestString));
    auto request = parser.GetNextRequest();
    REQUIRE(request.is_initialized());
    REQUIRE(request->GetMethod() == HttpMethod::GET);
    REQUIRE(request->GetUri().GetPath() == "/path/file.html");
    REQUIRE(request->GetVersion() == HttpVersion::Version1_0());
    REQUIRE(request->GetCookie("theme")->GetValue() == "light");
    REQUIRE(request->GetCookie("sessionToken")->GetValue() == "abc123");
  }
}
