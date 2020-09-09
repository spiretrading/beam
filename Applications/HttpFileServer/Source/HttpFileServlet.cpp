#include "HttpFileServer/HttpFileServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>

using namespace Beam;
using namespace Beam::HttpFileServer;
using namespace Beam::IO;
using namespace Beam::WebServices;

HttpFileServlet::HttpFileServlet()
  : m_fileStore("webapp") {}

HttpFileServlet::~HttpFileServlet() {
  Close();
}

std::vector<HttpRequestSlot> HttpFileServlet::GetSlots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(ServeIndex(m_fileStore));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    std::bind(&HttpFileServlet::OnServeFile, this, std::placeholders::_1));
  return slots;
}

void HttpFileServlet::Close() {
  m_openState.Close();
}

HttpResponse HttpFileServlet::OnServeFile(const HttpRequest& request) {
  return m_fileStore.Serve(request);
}
