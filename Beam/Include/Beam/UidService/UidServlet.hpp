#ifndef BEAM_UID_SERVLET_HPP
#define BEAM_UID_SERVLET_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/UidService/UidServices.hpp"

namespace Beam::UidService {

  /**
   * Provides unique ids to clients.
   * @param <C> The container instantiating this servlet.
   * @param <D> The type of data store to use.
   */
  template<typename C, typename D>
  class UidServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of UidDataStore used. */
      using UidDataStore = GetTryDereferenceType<D>;

      /**
       * Constructs a UidServlet.
       * @param dataStore The data store to use.
       */
      template<typename DF>
      UidServlet(DF&& dataStore);

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void Close();

    private:
      GetOptionalLocalPtr<D> m_dataStore;
      IO::OpenState m_openState;

      UidServlet(const UidServlet&) = delete;
      UidServlet& operator =(const UidServlet&) = delete;
      std::uint64_t OnReserveUidsRequest(ServiceProtocolClient& client,
        std::uint64_t blockSize);
  };

  template<typename D>
  struct MetaUidServlet {
    static constexpr auto SupportsParallelism = true;
    using Session = NullType;
    template<typename C>
    struct apply {
      using type = UidServlet<C, D>;
    };
  };

  template<typename C, typename D>
  template<typename DF>
  UidServlet<C, D>::UidServlet(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename C, typename D>
  void UidServlet<C, D>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterUidServices(Store(slots));
    ReserveUidsService::AddSlot(Store(slots), std::bind(
      &UidServlet::OnReserveUidsRequest, this, std::placeholders::_1,
      std::placeholders::_2));
  }

  template<typename C, typename D>
  void UidServlet<C, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename D>
  std::uint64_t UidServlet<C, D>::OnReserveUidsRequest(
      ServiceProtocolClient& client, std::uint64_t blockSize) {
    auto uid = std::uint64_t();
    m_dataStore->WithTransaction([&] {
      uid = m_dataStore->Reserve(blockSize);
    });
    return uid;
  }
}

#endif
