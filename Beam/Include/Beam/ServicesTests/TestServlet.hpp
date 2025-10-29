#ifndef BEAM_TEST_SERVLET_HPP
#define BEAM_TEST_SERVLET_HPP
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/ServicesTests/TestServices.hpp"

namespace Beam::Tests {

  /**
   * Implements a ServiceProtocolServlet for the test services.
   * @tparam C The type of ServiceProtocolServletContainer instantiating the
   *        Servlet.
   */
  template<typename C>
  class TestServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(ServiceProtocolClient& client);
      void handle_closed(ServiceProtocolClient& client);
      void close();

    private:
      bool m_void_service_requested;

      void on_void_request(
        RequestToken<ServiceProtocolClient, VoidService>& request, int n);
  };

  struct MetaTestServlet {
    using Session = NullSession;

    template<typename C>
    struct apply {
      using type = TestServlet<C>;
    };
  };

  template<typename C>
  void TestServlet<C>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    register_test_services(out(slots));
    VoidService::add_request_slot(
      out(slots), std::bind_front(&TestServlet::on_void_request, this));
  }

  template<typename C>
  void TestServlet<C>::handle_accept(ServiceProtocolClient& client) {}

  template<typename C>
  void TestServlet<C>::handle_closed(ServiceProtocolClient& client) {}

  template<typename C>
  void TestServlet<C>::close() {}

  template<typename C>
  void TestServlet<C>::on_void_request(
      RequestToken<ServiceProtocolClient, VoidService>& request, int n) {
    m_void_service_requested = true;
    request.set();
  }
}

#endif
