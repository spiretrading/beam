#ifndef BEAM_WEB_SOCKET_ECHO_SERVLET_HPP
#define BEAM_WEB_SOCKET_ECHO_SERVLET_HPP
#include <iostream>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>

namespace Beam::WebSocketEchoServer {

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
      std::vector<WebSocketSlot> GetWebSocketSlots();

      void Close();

    private:
      Beam::IO::OpenState m_openState;

      void OnUpgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
  };

  struct MetaWebSocketEchoServlet {
    template<typename C>
    struct apply {
      using type = WebSocketEchoServlet<C>;
    };
  };

  template<typename C>
  WebSocketEchoServlet<C>::~WebSocketEchoServlet() {
    Close();
  }

  template<typename C>
  std::vector<typename WebSocketEchoServlet<C>::WebSocketSlot>
      WebSocketEchoServlet<C>::GetWebSocketSlots() {
    auto slots = std::vector<WebSocketSlot>();
    slots.emplace_back(
      Beam::WebServices::MatchAny(Beam::WebServices::HttpMethod::GET),
      std::bind(&WebSocketEchoServlet::OnUpgrade, this, std::placeholders::_1,
      std::placeholders::_2));
    return slots;
  }

  template<typename C>
  void WebSocketEchoServlet<C>::Close() {
    m_openState.Close();
  }

  template<typename C>
  void WebSocketEchoServlet<C>::OnUpgrade(
      const Beam::WebServices::HttpRequest& request,
      std::unique_ptr<WebSocketChannel> channel) {
    Beam::Routines::Spawn([=, channel = std::move(channel)] {
      while(true) {
        auto buffer = typename WebSocketChannel::Reader::Buffer();
        channel->GetReader().Read(Beam::Store(buffer));
        std::cout << buffer << std::endl;
        channel->GetWriter().Write(buffer);
      }
    });
  }
}

#endif
