#include "Beam/WebServicesTests/HttpRequestParserTester.hpp"
#include "Beam/WebServices/HttpRequestParser.hpp"

using namespace Beam;
using namespace Beam::WebServices;
using namespace Beam::WebServices::Tests;
using namespace boost;
using namespace std;

void HttpRequestParserTester::TestValidRequest() {
  HttpRequestParser parser;
  auto request = "GET /home/ ";
  parser.Feed(request, strlen(request));
}
