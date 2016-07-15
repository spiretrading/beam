#include "HttpFileServer/HttpFileServer/HttpFileServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>

using namespace Beam;
using namespace Beam::HttpFileServer;
using namespace Beam::IO;
using namespace Beam::WebServices;
using namespace std;

HttpFileServlet::HttpFileServlet()
    : m_fileStore{"webapp"} {}

HttpFileServlet::~HttpFileServlet() {
  Close();
}

vector<HttpRequestSlot> HttpFileServlet::GetSlots() {
  vector<HttpRequestSlot> slots;
  slots.emplace_back(ServeIndex(m_fileStore));
  slots.emplace_back(MatchAny(HttpMethod::GET),
    bind(&HttpFileServlet::OnServeFile, this, std::placeholders::_1));
  return slots;
}

void HttpFileServlet::Open() {
  if(m_openState.SetOpening()) {
    return;
  }
  m_openState.SetOpen();
}

void HttpFileServlet::Close() {
  if(m_openState.SetClosing()) {
    return;
  }
  Shutdown();
}

void HttpFileServlet::Shutdown() {
  m_openState.SetClosed();
}

HttpResponse HttpFileServlet::OnServeFile(const HttpRequest& request) {
  return m_fileStore.Serve(request);
}
