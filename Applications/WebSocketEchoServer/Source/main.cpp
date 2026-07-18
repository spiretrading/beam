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

  /** Implements a web servlet that echo's messages. */
  template<typename C>
  class WebSocketEchoServlet {
    public:
      using Container = C;
      using WebSocketChannel = typename Container::WebSocketChannel;
      using WebSocketSlot = typename Container::WebSocketSlot;

      /** Constructs a WebSocketEchoServlet. */
      WebSocketEchoServlet() = default;

      ~WebSocketEchoServlet();

      /** Returns the web socket upgrade slots. */
      std::vector<WebSocketSlot> get_web_socket_slots();

      void close();

    private:
      OpenState m_open_state;

      void on_upgrade(
        const HttpRequest& request, std::unique_ptr<WebSocketChannel> channel);
  };

  struct MetaWebSocketEchoServlet {
    template<typename C>
    struct apply {
      using type = WebSocketEchoServlet<C>;
    };
  };

  template<typename C>
  WebSocketEchoServlet<C>::~WebSocketEchoServlet() {
    close();
  }

  template<typename C>
  std::vector<typename WebSocketEchoServlet<C>::WebSocketSlot>
      WebSocketEchoServlet<C>::get_web_socket_slots() {
    auto slots = std::vector<WebSocketSlot>();
    slots.emplace_back(match_any(HttpMethod::GET),
      std::bind_front(&WebSocketEchoServlet::on_upgrade, this));
    return slots;
  }

  template<typename C>
  void WebSocketEchoServlet<C>::close() {
    m_open_state.close();
  }

  template<typename C>
  void WebSocketEchoServlet<C>::on_upgrade(
      const HttpRequest& request, std::unique_ptr<WebSocketChannel> channel) {
    spawn([=, channel = std::move(channel)] {
      while(true) {
        auto buffer = SharedBuffer();
        channel->get_reader().read(out(buffer));
        std::cout << buffer << std::endl;
        channel->get_writer().write(buffer);
      }
    });
  }

  using WebSocketEchoServletContainer =
    HttpServletContainer<MetaWebSocketEchoServlet, TcpServerSocket>;
}

int main(int argc, const char** argv) {
  try {
    auto config =
      parse_command_line(argc, argv, "1.0-r" WEB_SOCKET_ECHO_SERVER_VERSION
        "\nCopyright (C) 2026 Spire Trading Inc.");
    auto interface = extract<IpAddress>(config, "interface");
    auto server = WebSocketEchoServletContainer(init(), init(interface));
    wait_for_kill_event();
  } catch(...) {
    report_current_exception();
    return -1;
  }
  return 0;
}
