#ifndef BEAM_TESTSERVLET_HPP
#define BEAM_TESTSERVLET_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/ServicesTests/TestServices.hpp"

namespace Beam {
namespace Services {
namespace Tests {

  /*! \class TestServlet
      \brief Implements a ServiceProtocolServlet for the test services.
      \tparam ContainerType The type of ServiceProtocolServletContainer
                            instantiating the Servlet.
   */
  template<typename ContainerType>
  class TestServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! Constructs a TestServlet.
      TestServlet();

      void RegisterServices(Out<ServiceSlots<ServiceProtocolClient>> slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      bool m_voidServiceRequested;

      void OnVoidRequest(RequestToken<ServiceProtocolClient, VoidService>&
        request, int n);
  };

  struct MetaTestServlet {
    using Session = NullType;
    template<typename ContainerType>
    struct apply {
      using type = TestServlet<ContainerType>;
    };
  };

  template<typename ContainerType>
  TestServlet<ContainerType>::TestServlet() {}

  template<typename ContainerType>
  void TestServlet<ContainerType>::RegisterServices(Out<ServiceSlots<
      ServiceProtocolClient>> slots) {
    RegisterTestServices(Store(slots));
    VoidService::AddRequestSlot(Store(slots),
      std::bind(&TestServlet::OnVoidRequest, this, std::placeholders::_1,
      std::placeholders::_2));
  }

  template<typename ContainerType>
  void TestServlet<ContainerType>::HandleClientAccepted(
    ServiceProtocolClient& client) {}

  template<typename ContainerType>
  void TestServlet<ContainerType>::HandleClientClosed(
    ServiceProtocolClient& client) {}

  template<typename ContainerType>
  void TestServlet<ContainerType>::Close() {}

  template<typename ContainerType>
  void TestServlet<ContainerType>::OnVoidRequest(
      RequestToken<ServiceProtocolClient, VoidService>& request, int n) {
    m_voidServiceRequested = true;
    request.SetResult();
  }
}
}

  template<typename ServiceProtocolClientType>
  struct ImplementsConcept<Services::Tests::TestServlet<
    ServiceProtocolClientType>, Services::ServiceProtocolServlet<
    ServiceProtocolClientType>> : std::true_type {};
}

#endif
