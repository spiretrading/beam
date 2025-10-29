#include "HttpFileServer/HttpFileServlet.hpp"
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>

using namespace Beam;

HttpFileServlet::HttpFileServlet()
  : m_file_store("web_app") {}

HttpFileServlet::~HttpFileServlet() {
  close();
}

std::vector<HttpRequestSlot> HttpFileServlet::get_slots() {
  auto slots = std::vector<HttpRequestSlot>();
  slots.emplace_back(match_any(HttpMethod::GET),
    std::bind_front(&HttpFileServlet::on_serve_file, this));
  return slots;
}

void HttpFileServlet::close() {
  m_open_state.close();
}

HttpResponse HttpFileServlet::on_serve_file(const HttpRequest& request) {
  return m_file_store.serve(request);
}
