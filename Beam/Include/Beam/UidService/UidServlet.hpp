#ifndef BEAM_UIDSERVLET_HPP
#define BEAM_UIDSERVLET_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/UidService/UidServices.hpp"

namespace Beam {
namespace UidService {

  /*! \class UidServlet
      \brief Provides unique ids to clients.
      \tparam ContainerType The container instantiating this servlet.
      \tparam UidDataStoreType The type of data store to use.
   */
  template<typename ContainerType, typename UidDataStoreType>
  class UidServlet : private boost::noncopyable {
    public:
      typedef ContainerType Container;
      typedef typename Container::ServiceProtocolClient ServiceProtocolClient;

      //! The type of UidDataStore used.
      typedef typename TryDereferenceType<UidDataStoreType>::type UidDataStore;

      //! Constructs a UidServlet.
      /*!
        \param dataStore The data store to use.
      */
      template<typename DataStoreForward>
      UidServlet(DataStoreForward&& dataStore);

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void Open();

      void Close();

    private:
      typename OptionalLocalPtr<UidDataStoreType>::type m_dataStore;
      IO::OpenState m_openState;

      void Shutdown();
      std::uint64_t OnReserveUidsRequest(ServiceProtocolClient& client,
        std::uint64_t blockSize);
  };

  template<typename UidDataStoreType>
  struct MetaUidServlet {
    static constexpr bool SupportsParallelism = true;
    using Session = NullType;
    template<typename ContainerType>
    struct apply {
      typedef UidServlet<ContainerType, UidDataStoreType> type;
    };
  };

  template<typename ContainerType, typename UidDataStoreType>
  template<typename DataStoreForward>
  UidServlet<ContainerType, UidDataStoreType>::UidServlet(
      DataStoreForward&& dataStore)
      : m_dataStore(std::forward<DataStoreForward>(dataStore)) {}

  template<typename ContainerType, typename UidDataStoreType>
  void UidServlet<ContainerType, UidDataStoreType>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterUidServices(Store(slots));
    ReserveUidsService::AddSlot(Store(slots), std::bind(
      &UidServlet::OnReserveUidsRequest, this, std::placeholders::_1,
      std::placeholders::_2));
  }

  template<typename ContainerType, typename UidDataStoreType>
  void UidServlet<ContainerType, UidDataStoreType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_dataStore->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename UidDataStoreType>
  void UidServlet<ContainerType, UidDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename UidDataStoreType>
  void UidServlet<ContainerType, UidDataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename UidDataStoreType>
  std::uint64_t UidServlet<ContainerType, UidDataStoreType>::
      OnReserveUidsRequest(ServiceProtocolClient& client,
      std::uint64_t blockSize) {
    std::uint64_t uid;
    m_dataStore->WithTransaction(
      [&] {
        uid = this->m_dataStore->Reserve(blockSize);
      });
    return uid;
  }
}
}

#endif
