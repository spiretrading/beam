#ifndef BEAM_WEB_SOCKET_ECHO_SERVLET_HPP
#define BEAM_WEB_SOCKET_ECHO_SERVLET_HPP
#include <iostream>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>

namespace Beam {

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
}

#endif
