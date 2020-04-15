#include <doctest/doctest.h>
#include "Beam/Stomp/StompFrameParser.hpp"

using namespace Beam;
using namespace Beam::Stomp;

TEST_SUITE("StompFrameParser") {
  TEST_CASE("new_line_character") {
    auto parser = StompFrameParser();
    auto contents = std::string(
      "SEND\n"
      "destination:/test/server\n"
      "host:testhost\n"
      "content-length:11\n\n"
      "hello world\n");
    parser.Feed(contents.c_str(), contents.size());
    auto frame = parser.GetNextFrame();
    REQUIRE(frame.is_initialized());
    REQUIRE(frame->GetCommand() == StompCommand::SEND);
    auto destinationHeader = frame->FindHeader("destination");
    REQUIRE(destinationHeader.is_initialized());
    REQUIRE(*destinationHeader == "/test/server");
    auto hostHeader = frame->FindHeader("host");
    REQUIRE(hostHeader.is_initialized());
    REQUIRE(*hostHeader == "testhost");
    auto body = frame->GetBody();
    auto message = std::string(body.GetData(), body.GetSize());
    REQUIRE(message == "hello world");
  }

  TEST_CASE("carriage_and_new_line_character") {
    auto parser = StompFrameParser();
    auto contents = std::string(
      "SEND\r\n"
      "destination:/test/server\r\n"
      "host:testhost\r\n"
      "content-length:11\r\n\r\n"
      "hello world\r\n");
    parser.Feed(contents.c_str(), contents.size());
    auto frame = parser.GetNextFrame();
    REQUIRE(frame.is_initialized());
    REQUIRE(frame->GetCommand() == StompCommand::SEND);
    auto destinationHeader = frame->FindHeader("destination");
    REQUIRE(destinationHeader.is_initialized());
    REQUIRE(*destinationHeader == "/test/server");
    auto hostHeader = frame->FindHeader("host");
    REQUIRE(hostHeader.is_initialized());
    REQUIRE(*hostHeader == "testhost");
    auto body = frame->GetBody();
    auto message = std::string(body.GetData(), body.GetSize());
    REQUIRE(message == "hello world");
  }

  TEST_CASE("eol_frame") {
    auto parser = StompFrameParser();
    auto contents = std::string(
      "\r\n\r\n\n\nSEND\r\n"
      "destination:/test/server\r\n"
      "host:testhost\r\n"
      "content-length:11\r\n\r\n"
      "hello world\r\n\r\n\n"
      "SEND\r\n"
      "destination:/dev/null\r\n\r\n"
      "goodbye sky");
    contents.push_back('\0');
    contents += "\r\n";
    parser.Feed(contents.c_str(), contents.size());
    {
      auto frame = parser.GetNextFrame();
      REQUIRE(frame.is_initialized());
      REQUIRE(frame->GetCommand() == StompCommand::SEND);
      auto destinationHeader = frame->FindHeader("destination");
      REQUIRE(destinationHeader.is_initialized());
      REQUIRE(*destinationHeader == "/test/server");
      auto hostHeader = frame->FindHeader("host");
      REQUIRE(hostHeader.is_initialized());
      REQUIRE(*hostHeader == "testhost");
      auto body = frame->GetBody();
      auto message = std::string(body.GetData(), body.GetSize());
      REQUIRE(message == "hello world");
    }
    {
      auto frame = parser.GetNextFrame();
      REQUIRE(frame.is_initialized());
      REQUIRE(frame->GetCommand() == StompCommand::SEND);
      auto destinationHeader = frame->FindHeader("destination");
      REQUIRE(destinationHeader.is_initialized());
      REQUIRE(*destinationHeader == "/dev/null");
      auto body = frame->GetBody();
      auto message = std::string(body.GetData(), body.GetSize());
      REQUIRE(message == "goodbye sky");
    }
  }

  TEST_CASE("escape_characters") {
    auto parser = StompFrameParser();
    auto contents = std::string(
      "SEND\r\n"
      "destination:/test/server\r\n"
      "a\\nb:a\\cb\r\n"
      "content-length:0\r\n\r\n\r\n");
    parser.Feed(contents.c_str(), contents.size());
    auto frame = parser.GetNextFrame();
    REQUIRE(frame.is_initialized());
    auto header = frame->FindHeader("a\nb");
    REQUIRE(header.is_initialized());
    REQUIRE(*header == "a:b");
  }
}
