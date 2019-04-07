#ifndef BEAM_WEBSOCKETECHOSERVLET_HPP
#define BEAM_WEBSOCKETECHOSERVLET_HPP
#include <iostream>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/WebServices/HttpRequestSlot.hpp>
#include <Beam/WebServices/HttpServerPredicates.hpp>
#include <Beam/WebServices/HttpUpgradeSlot.hpp>
#include <boost/noncopyable.hpp>

namespace Beam {
namespace WebSocketEchoServer {

  /*! \class WebSocketEchoServlet
      \brief Implements a web servlet that echo's messages.
   */
  template<typename ContainerType>
  class WebSocketEchoServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using WebSocketChannel = typename Container::WebSocketChannel;
      using WebSocketSlot = typename Container::WebSocketSlot;

      //! Constructs a WebSocketEchoServlet.
      WebSocketEchoServlet() = default;

      ~WebSocketEchoServlet();

      //! Returns the web socket upgrade slots.
      std::vector<WebSocketSlot> GetWebSocketSlots();

      void Open();

      void Close();

    private:
      Beam::IO::OpenState m_openState;

      void Shutdown();
      void OnUpgrade(const Beam::WebServices::HttpRequest& request,
        std::unique_ptr<WebSocketChannel> channel);
  };

  struct MetaWebSocketEchoServlet {
    template<typename ContainerType>
    struct apply {
      using type = WebSocketEchoServlet<ContainerType>;
    };
  };

  template<typename ContainerType>
  WebSocketEchoServlet<ContainerType>::~WebSocketEchoServlet() {
    Close();
  }

  template<typename ContainerType>
  std::vector<typename WebSocketEchoServlet<ContainerType>::WebSocketSlot>
      WebSocketEchoServlet<ContainerType>::GetWebSocketSlots() {
    std::vector<WebSocketSlot> slots;
    slots.emplace_back(
      Beam::WebServices::MatchAny(Beam::WebServices::HttpMethod::GET),
      std::bind(&WebSocketEchoServlet::OnUpgrade, this, std::placeholders::_1,
      std::placeholders::_2));
    return slots;
  }

  template<typename ContainerType>
  void WebSocketEchoServlet<ContainerType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType>
  void WebSocketEchoServlet<ContainerType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType>
  void WebSocketEchoServlet<ContainerType>::Shutdown() {
    m_openState.SetClosed();
  }

  template<typename ContainerType>
  void WebSocketEchoServlet<ContainerType>::OnUpgrade(
      const Beam::WebServices::HttpRequest& request,
      std::unique_ptr<WebSocketChannel> channel) {
    Beam::Routines::Spawn(
      [=, channel = std::move(channel)] {
        while(true) {
          typename WebSocketChannel::Reader::Buffer buffer;
          channel->GetReader().Read(Beam::Store(buffer));
          std::cout << buffer << std::endl;
          channel->GetWriter().Write(buffer);
        }
      });
  }
}
}

#endif
