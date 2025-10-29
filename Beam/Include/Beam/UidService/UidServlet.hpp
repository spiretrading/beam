#ifndef BEAM_UID_SERVLET_HPP
#define BEAM_UID_SERVLET_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/UidService/UidDataStore.hpp"
#include "Beam/UidService/UidServices.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Provides unique ids to clients.
   * @tparam C The container instantiating this servlet.
   * @tparam D The type of data store to use.
   */
  template<typename C, typename D> requires IsUidDataStore<dereference_t<D>>
  class UidServlet {
    public:

      /** The type of UidDataStore used. */
      using UidDataStore = dereference_t<D>;

      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /**
       * Constructs a UidServlet.
       * @param data_store The data store to use.
       */
      template<Initializes<D> DF>
      explicit UidServlet(DF&& data_store);

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void close();

    private:
      local_ptr_t<D> m_data_store;
      OpenState m_open_state;

      UidServlet(const UidServlet&) = delete;
      UidServlet& operator =(const UidServlet&) = delete;
      std::uint64_t on_reserve_uids_request(
        ServiceProtocolClient& client, std::uint64_t block_size);
  };

  template<typename D> requires IsUidDataStore<dereference_t<D>>
  struct MetaUidServlet {
    static constexpr auto SUPPORTS_PARALLELISM = true;
    using Session = NullSession;

    template<typename C>
    struct apply {
      using type = UidServlet<C, D>;
    };
  };

  template<typename C, typename D> requires IsUidDataStore<dereference_t<D>>
  template<Initializes<D> DF>
  UidServlet<C, D>::UidServlet(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<typename C, typename D> requires IsUidDataStore<dereference_t<D>>
  void UidServlet<C, D>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    register_uid_services(out(slots));
    ReserveUidsService::add_slot(
      out(slots), std::bind_front(&UidServlet::on_reserve_uids_request, this));
  }

  template<typename C, typename D> requires IsUidDataStore<dereference_t<D>>
  void UidServlet<C, D>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_data_store->close();
    m_open_state.close();
  }

  template<typename C, typename D> requires IsUidDataStore<dereference_t<D>>
  std::uint64_t UidServlet<C, D>::on_reserve_uids_request(
      ServiceProtocolClient& client, std::uint64_t block_size) {
    return m_data_store->with_transaction([&] {
      return m_data_store->reserve(block_size);
    });
  }
}

#endif
