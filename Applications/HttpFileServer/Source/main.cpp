module;
#include "Prelude.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

module Beam;

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {

  /** Implements a web servlet for static files. */
  class HttpFileServlet {
    public:

      /** Constructs a HttpFileServlet. */
      HttpFileServlet();

      ~HttpFileServlet();

      std::vector<HttpRequestSlot> get_slots();
      void close();

    private:
      FileStore m_file_store;
      OpenState m_open_state;

      HttpFileServlet(const HttpFileServlet&) = delete;
      HttpFileServlet& operator =(const HttpFileServlet&) = delete;
      HttpResponse on_serve_file(const HttpRequest& request);
  };

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

  using HttpFileServletContainer =
    HttpServletContainer<HttpFileServlet, TcpServerSocket>;
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" HTTP_FILE_SERVER_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto server = HttpFileServletContainer(init(), init(interface));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
