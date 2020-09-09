#ifndef BEAM_SERVLET_TEMPLATE_SERVLET_HPP
#define BEAM_SERVLET_TEMPLATE_SERVLET_HPP
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "ServletTemplate/Services.hpp"

namespace Beam {

  /**
   * Basic template used to develop a servlet.
   * @param <C> The container instantiating this servlet.
   */
  template<typename C>
  class ServletTemplateServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** Constructs a ServletTemplateServlet. */
      ServletTemplateServlet();

      void RegisterServices(
        Out<Services::ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      struct EchoEntry {
        ServiceProtocolClient* m_client;
        std::string m_message;
        int m_rate;
        int m_countdown;
        int m_messages;
      };
      Threading::TimerThreadPool m_timerThreadPool;
      SynchronizedVector<EchoEntry> m_echoEntries;
      std::unique_ptr<Threading::LiveTimer> m_echoTimer;
      IO::OpenState m_openState;
      RoutineTaskQueue m_taskQueue;

      void OnEchoTimer(Threading::Timer::Result result);
      int OnEchoRequest(ServiceProtocolClient& client,
        const std::string& message, int rate);
  };

  struct MetaServletTemplateServlet {
    using Session = NullType;
    template<typename C>
    struct apply {
      using type = ServletTemplateServlet<C>;
    };
  };

  template<typename C>
  ServletTemplateServlet<C>::ServletTemplateServlet() {
    m_echoTimer = std::make_unique<Threading::LiveTimer>(
      boost::posix_time::milliseconds(100), Ref(m_timerThreadPool));
    m_echoTimer->GetPublisher().Monitor(
      m_taskQueue.GetSlot<Threading::Timer::Result>(
      std::bind(&ServletTemplateServlet::OnEchoTimer, this,
      std::placeholders::_1)));
    m_echoTimer->Start();
  }

  template<typename C>
  void ServletTemplateServlet<C>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterServletTemplateServices(Store(slots));
    RegisterServletTemplateMessages(Store(slots));
    EchoService::AddSlot(Store(slots), std::bind(
      &ServletTemplateServlet::OnEchoRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
  }

  template<typename C>
  void ServletTemplateServlet<C>::HandleClientClosed(
      ServiceProtocolClient& client) {
    m_echoEntries.RemoveIf([&] (const auto& entry) {
      return entry.m_client == &client;
    });
  }

  template<typename C>
  void ServletTemplateServlet<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_echoTimer->Cancel();
    m_openState.Close();
  }

  template<typename C>
  void ServletTemplateServlet<C>::OnEchoTimer(Threading::Timer::Result result) {
    if(result != Threading::Timer::Result::EXPIRED) {
      return;
    }
    m_echoEntries.ForEach([&] (auto& entry) {
      if(entry.m_messages == 0) {
        --entry.m_countdown;
        if(entry.m_countdown == 0) {
          auto timestamp = boost::posix_time::microsec_clock::universal_time();
          Beam::Services::SendRecordMessage<EchoMessage>(*entry.m_client,
            timestamp, entry.m_message);
          entry.m_countdown = 10 / entry.m_rate;
        }
      } else {
        auto timestamp = boost::posix_time::microsec_clock::universal_time();
        for(auto i = 0; i < entry.m_messages; ++i) {
          Beam::Services::SendRecordMessage<EchoMessage>(*entry.m_client,
            timestamp, entry.m_message);
        }
      }
    });
    m_echoTimer->Start();
  }

  template<typename C>
  int ServletTemplateServlet<C>::OnEchoRequest(ServiceProtocolClient& client,
      const std::string& message, int rate) {
    auto [countdown, messages] = [&] {
      if(rate > 10) {
        return std::tuple(0, rate / 10);
      } else {
        return std::tuple(10 / rate, 0);
      }
    }();
    auto entry = EchoEntry{&client, message, rate, countdown, messages};
    m_echoEntries.PushBack(entry);
    return 0;
  }
}

#endif
