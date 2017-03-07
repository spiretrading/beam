#include "Beam/StompTests/StompFrameParserTester.hpp"
#include "Beam/Stomp/StompFrameParser.hpp"

using namespace Beam;
using namespace Beam::Stomp;
using namespace Beam::Stomp::Tests;
using namespace std;

void StompFrameParserTester::TestNewLineCharacter() {
  StompFrameParser parser;
  string contents =
    "SEND\n"
    "destination:/test/server\n"
    "host:testhost\n"
    "content-length:11\n\n"
    "hello world\n";
  parser.Feed(contents.c_str(), contents.size());
  auto frame = parser.GetNextFrame();
  CPPUNIT_ASSERT(frame.is_initialized());
  CPPUNIT_ASSERT(frame->GetCommand() == StompCommand::SEND);
  auto destinationHeader = frame->FindHeader("destination");
  CPPUNIT_ASSERT(destinationHeader.is_initialized());
  CPPUNIT_ASSERT(*destinationHeader == "/test/server");
  auto hostHeader = frame->FindHeader("host");
  CPPUNIT_ASSERT(hostHeader.is_initialized());
  CPPUNIT_ASSERT(*hostHeader == "testhost");
  auto body = frame->GetBody();
  string message{body.GetData(), body.GetSize()};
  CPPUNIT_ASSERT(message == "hello world");
}

void StompFrameParserTester::TestCarriageAndNewLineCharacter() {
  StompFrameParser parser;
  string contents =
    "SEND\r\n"
    "destination:/test/server\r\n"
    "host:testhost\r\n"
    "content-length:11\r\n\r\n"
    "hello world\r\n";
  parser.Feed(contents.c_str(), contents.size());
  auto frame = parser.GetNextFrame();
  CPPUNIT_ASSERT(frame.is_initialized());
  CPPUNIT_ASSERT(frame->GetCommand() == StompCommand::SEND);
  auto destinationHeader = frame->FindHeader("destination");
  CPPUNIT_ASSERT(destinationHeader.is_initialized());
  CPPUNIT_ASSERT(*destinationHeader == "/test/server");
  auto hostHeader = frame->FindHeader("host");
  CPPUNIT_ASSERT(hostHeader.is_initialized());
  CPPUNIT_ASSERT(*hostHeader == "testhost");
  auto body = frame->GetBody();
  string message{body.GetData(), body.GetSize()};
  CPPUNIT_ASSERT(message == "hello world");
}

void StompFrameParserTester::TestEolFrame() {
  StompFrameParser parser;
  string contents =
    "\r\n\r\n\n\nSEND\r\n"
    "destination:/test/server\r\n"
    "host:testhost\r\n"
    "content-length:11\r\n\r\n"
    "hello world\r\n\r\n\n"
    "SEND\r\n"
    "destination:/dev/null\r\n\r\n"
    "goodbye sky";
  contents.push_back('\0');
  contents += "\r\n";
  parser.Feed(contents.c_str(), contents.size());
  {
    auto frame = parser.GetNextFrame();
    CPPUNIT_ASSERT(frame.is_initialized());
    CPPUNIT_ASSERT(frame->GetCommand() == StompCommand::SEND);
    auto destinationHeader = frame->FindHeader("destination");
    CPPUNIT_ASSERT(destinationHeader.is_initialized());
    CPPUNIT_ASSERT(*destinationHeader == "/test/server");
    auto hostHeader = frame->FindHeader("host");
    CPPUNIT_ASSERT(hostHeader.is_initialized());
    CPPUNIT_ASSERT(*hostHeader == "testhost");
    auto body = frame->GetBody();
    string message{body.GetData(), body.GetSize()};
    CPPUNIT_ASSERT(message == "hello world");
  }
  {
    auto frame = parser.GetNextFrame();
    CPPUNIT_ASSERT(frame.is_initialized());
    CPPUNIT_ASSERT(frame->GetCommand() == StompCommand::SEND);
    auto destinationHeader = frame->FindHeader("destination");
    CPPUNIT_ASSERT(destinationHeader.is_initialized());
    CPPUNIT_ASSERT(*destinationHeader == "/dev/null");
    auto body = frame->GetBody();
    string message{body.GetData(), body.GetSize()};
    CPPUNIT_ASSERT(message == "goodbye sky");
  }
}

void StompFrameParserTester::TestEscapeCharacters() {
  StompFrameParser parser;
  string contents =
    "SEND\r\n"
    "destination:/test/server\r\n"
    "a\\nb:a\\cb\r\n"
    "content-length:0\r\n\r\n\r\n";
  parser.Feed(contents.c_str(), contents.size());
  auto frame = parser.GetNextFrame();
  CPPUNIT_ASSERT(frame.is_initialized());
  auto header = frame->FindHeader("a\nb");
  CPPUNIT_ASSERT(header.is_initialized());
  CPPUNIT_ASSERT(*header == "a:b");
}
