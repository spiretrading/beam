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
