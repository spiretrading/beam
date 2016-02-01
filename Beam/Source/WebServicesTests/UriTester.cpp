#include "Avalon/WebServicesTests/UriTester.hpp"
#include "Avalon/WebServices/Uri.hpp"

using namespace Avalon;
using namespace Avalon::WebServices;
using namespace Avalon::WebServices::Tests;
using namespace std;

void UriTester::TestSchemeOnly() {
  Uri uri("tcp:");
  CPPUNIT_ASSERT(uri.GetScheme() == "tcp");
  CPPUNIT_ASSERT(uri.GetUsername().empty());
  CPPUNIT_ASSERT(uri.GetPassword().empty());
  CPPUNIT_ASSERT(uri.GetHostname().empty());
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "tcp:");
}

void UriTester::TestHostnameOnly() {
  Uri uri("//localhost");
  CPPUNIT_ASSERT(uri.GetScheme().empty());
  CPPUNIT_ASSERT(uri.GetUsername().empty());
  CPPUNIT_ASSERT(uri.GetPassword().empty());
  CPPUNIT_ASSERT(uri.GetHostname() == "localhost");
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "//localhost");
}

void UriTester::TestSchemeAndEmptyHostname() {
  Uri uri("tcp:/");
  CPPUNIT_ASSERT(uri.GetScheme() == "tcp");
  CPPUNIT_ASSERT(uri.GetUsername().empty());
  CPPUNIT_ASSERT(uri.GetPassword().empty());
  CPPUNIT_ASSERT(uri.GetHostname().empty());
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath() == "/");
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "tcp:/");
}

void UriTester::TestSchemeAndHostname() {
  Uri uri("tcp://localhost");
  CPPUNIT_ASSERT(uri.GetScheme() == "tcp");
  CPPUNIT_ASSERT(uri.GetUsername().empty());
  CPPUNIT_ASSERT(uri.GetPassword().empty());
  CPPUNIT_ASSERT(uri.GetHostname() == "localhost");
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "tcp://localhost");
}

void UriTester::TestUsernameOnly() {
  Uri uri("//a:@localhost");
  CPPUNIT_ASSERT(uri.GetScheme().empty());
  CPPUNIT_ASSERT(uri.GetUsername() == "a");
  CPPUNIT_ASSERT(uri.GetPassword().empty());
  CPPUNIT_ASSERT(uri.GetHostname() == "localhost");
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "//a:@localhost");
}

void UriTester::TestPasswordOnly() {
  Uri uri("//:b@localhost");
  CPPUNIT_ASSERT(uri.GetScheme().empty());
  CPPUNIT_ASSERT(uri.GetUsername().empty());
  CPPUNIT_ASSERT(uri.GetPassword() == "b");
  CPPUNIT_ASSERT(uri.GetHostname() == "localhost");
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "//:b@localhost");
}

void UriTester::TestUsernameAndPassword() {
  Uri uri("//a:b@localhost");
  CPPUNIT_ASSERT(uri.GetScheme().empty());
  CPPUNIT_ASSERT(uri.GetUsername() == "a");
  CPPUNIT_ASSERT(uri.GetPassword() == "b");
  CPPUNIT_ASSERT(uri.GetHostname() == "localhost");
  CPPUNIT_ASSERT(uri.GetPort() == 0);
  CPPUNIT_ASSERT(uri.GetPath().empty());
  CPPUNIT_ASSERT(uri.GetQuery().empty());
  CPPUNIT_ASSERT(uri.GetFragment().empty());
  CPPUNIT_ASSERT(uri.ToString() == "//a:b@localhost");
}
