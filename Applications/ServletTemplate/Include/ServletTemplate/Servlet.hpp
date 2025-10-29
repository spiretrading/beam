#ifndef BEAM_SERVLET_TEMPLATE_SERVLET_HPP
#define BEAM_SERVLET_TEMPLATE_SERVLET_HPP
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
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

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void handle_close(ServiceProtocolClient& client);
      void close();

    private:
      struct EchoEntry {
        ServiceProtocolClient* m_client;
        std::string m_message;
        int m_rate;
        int m_countdown;
        int m_messages;
      };
      SynchronizedVector<EchoEntry> m_echo_entries;
      std::unique_ptr<LiveTimer> m_echo_timer;
      OpenState m_open_state;
      RoutineTaskQueue m_tasks;

      void on_echo_timer(Timer::Result result);
      int on_echo_request(
        ServiceProtocolClient& client, const std::string& message, int rate);
  };

  struct MetaServletTemplateServlet {
    using Session = NullSession;
    template<typename C>
    struct apply {
      using type = ServletTemplateServlet<C>;
    };
  };

  template<typename C>
  ServletTemplateServlet<C>::ServletTemplateServlet() {
    m_echo_timer =
      std::make_unique<LiveTimer>(boost::posix_time::milliseconds(100));
    m_echo_timer->get_publisher().monitor(m_tasks.get_slot<Timer::Result>(
      std::bind_front(&ServletTemplateServlet::on_echo_timer, this)));
    m_echo_timer->start();
  }

  template<typename C>
  void ServletTemplateServlet<C>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    register_servlet_template_services(out(slots));
    register_servlet_template_messages(out(slots));
    EchoService::add_slot(out(slots),
      std::bind_front(&ServletTemplateServlet::on_echo_request, this));
  }

  template<typename C>
  void ServletTemplateServlet<C>::handle_close(ServiceProtocolClient& client) {
    m_echo_entries.erase_if([&] (const auto& entry) {
      return entry.m_client == &client;
    });
  }

  template<typename C>
  void ServletTemplateServlet<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_echo_timer->cancel();
    m_open_state.close();
  }

  template<typename C>
  void ServletTemplateServlet<C>::on_echo_timer(Timer::Result result) {
    if(result != Timer::Result::EXPIRED) {
      return;
    }
    m_echo_entries.for_each([&] (auto& entry) {
      if(entry.m_messages == 0) {
        --entry.m_countdown;
        if(entry.m_countdown == 0) {
          auto timestamp = boost::posix_time::microsec_clock::universal_time();
          send_record_message<EchoMessage>(
            *entry.m_client, timestamp, entry.m_message);
          entry.m_countdown = 10 / entry.m_rate;
        }
      } else {
        auto timestamp = boost::posix_time::microsec_clock::universal_time();
        for(auto i = 0; i < entry.m_messages; ++i) {
          send_record_message<EchoMessage>(
            *entry.m_client, timestamp, entry.m_message);
        }
      }
    });
    m_echo_timer->start();
  }

  template<typename C>
  int ServletTemplateServlet<C>::on_echo_request(
      ServiceProtocolClient& client, const std::string& message, int rate) {
    auto [countdown, messages] = [&] {
      if(rate > 10) {
        return std::tuple(0, rate / 10);
      } else {
        return std::tuple(10 / rate, 0);
      }
    }();
    auto entry = EchoEntry(&client, message, rate, countdown, messages);
    m_echo_entries.push_back(entry);
    return 0;
  }
}

#endif
