#ifndef BEAM_REGISTRYSERVLET_HPP
#define BEAM_REGISTRYSERVLET_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/RegistryService/RegistryServices.hpp"
#include "Beam/RegistryService/RegistrySession.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class RegistryServlet
      \brief Provides access to a registry of resources and data.
      \tparam ContainerType The container instantiating this servlet.
      \tparam RegistryDataStoreType The type of data store to use.
   */
  template<typename ContainerType, typename RegistryDataStoreType>
  class RegistryServlet : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      //! The type of RegistryDataStore used.
      using RegistryDataStore = GetTryDereferenceType<RegistryDataStoreType>;

      //! Constructs a RegistryServlet.
      /*!
        \param dataStore The data store to use.
      */
      template<typename DataStoreForward>
      RegistryServlet(DataStoreForward&& dataStore);

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<RegistryDataStoreType> m_dataStore;
      IO::OpenState m_openState;

      void Shutdown();
      RegistryEntry OnLoadPathRequest(ServiceProtocolClient& client,
        const RegistryEntry& root, const std::string& path);
      RegistryEntry OnLoadParentRequest(ServiceProtocolClient& client,
        const RegistryEntry& registryEntry);
      std::vector<RegistryEntry> OnLoadChildrenRequest(
        ServiceProtocolClient& client, const RegistryEntry& registryEntry);
      RegistryEntry OnMakeDirectoryRequest(ServiceProtocolClient& client,
        const std::string& name, const RegistryEntry& parent);
      RegistryEntry OnCopyRequest(ServiceProtocolClient& client,
        const RegistryEntry& registryEntry, const RegistryEntry& destination);
      void OnMoveRequest(ServiceProtocolClient& client,
        const RegistryEntry& registryEntry, const RegistryEntry& destination);
      IO::SharedBuffer OnLoadValueRequest(ServiceProtocolClient& client,
        const RegistryEntry& registryEntry);
      RegistryEntry OnMakeValueRequest(ServiceProtocolClient& client,
        const std::string& name, const IO::SharedBuffer& value,
        const RegistryEntry& parent);
      RegistryEntry OnStoreValueRequest(ServiceProtocolClient& client,
        const std::string& name, const IO::SharedBuffer& value,
        const RegistryEntry& parent);
      void OnDeleteRequest(ServiceProtocolClient& client,
        const RegistryEntry& registryEntry);
  };

  template<typename RegistryDataStoreType>
  struct MetaRegistryServlet {
    static constexpr bool SupportsParallelism = true;
    using Session = RegistrySession;
    template<typename ContainerType>
    struct apply {
      using type = RegistryServlet<ContainerType, RegistryDataStoreType>;
    };
  };

  template<typename ContainerType, typename RegistryDataStoreType>
  template<typename DataStoreForward>
  RegistryServlet<ContainerType, RegistryDataStoreType>::RegistryServlet(
      DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    RegisterRegistryServices(Store(slots));
    LoadPathService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnLoadPathRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    LoadParentService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnLoadParentRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    LoadChildrenService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnLoadChildrenRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    MakeDirectoryService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnMakeDirectoryRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    CopyService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnCopyRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    MoveService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnMoveRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3));
    LoadValueService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnLoadValueRequest, this, std::placeholders::_1,
      std::placeholders::_2));
    MakeValueService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnMakeValueRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    StoreValueService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnStoreValueRequest, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    DeleteService::AddSlot(Store(slots), std::bind(
      &RegistryServlet::OnDeleteRequest, this, std::placeholders::_1,
      std::placeholders::_2));
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::Open() {
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

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::Shutdown() {
    m_dataStore->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnLoadPathRequest(ServiceProtocolClient& client,
      const RegistryEntry& root, const std::string& path) {
    RegistryEntry entry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedRoot = m_dataStore->Validate(root);
        entry = LoadRegistryEntry(*m_dataStore, validatedRoot, path);
      });
    return entry;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnLoadParentRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry) {
    RegistryEntry parent;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        parent = m_dataStore->LoadParent(validatedEntry);
      });
    return parent;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  std::vector<RegistryEntry>
      RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnLoadChildrenRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry) {
    std::vector<RegistryEntry> children;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        children = m_dataStore->LoadChildren(validatedEntry);
      });
    return children;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnMakeDirectoryRequest(ServiceProtocolClient& client,
      const std::string& name, const RegistryEntry& parent) {
    RegistryEntry newEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedParent = m_dataStore->Validate(parent);
        RegistryEntry validatedEntry(RegistryEntry::Type::DIRECTORY, -1, name,
          0);
        newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
      });
    return newEntry;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnCopyRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    RegistryEntry copiedEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        auto validatedDestination = m_dataStore->Validate(destination);
        copiedEntry = m_dataStore->Copy(validatedEntry, validatedDestination);
      });
    return copiedEntry;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::OnMoveRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry,
      const RegistryEntry& destination) {
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        auto validatedDestination = m_dataStore->Validate(destination);
        m_dataStore->Move(validatedEntry, validatedDestination);
      });
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  IO::SharedBuffer RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnLoadValueRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry) {
    IO::SharedBuffer value;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        value = m_dataStore->Load(validatedEntry);
      });
    return value;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnMakeValueRequest(ServiceProtocolClient& client, const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    RegistryEntry newEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedParent = m_dataStore->Validate(parent);
        RegistryEntry validatedEntry(RegistryEntry::Type::VALUE, -1, name, 0);
        newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
        m_dataStore->Store(newEntry, value);
      });
    return newEntry;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  RegistryEntry RegistryServlet<ContainerType, RegistryDataStoreType>::
      OnStoreValueRequest(ServiceProtocolClient& client,
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    RegistryEntry newEntry;
    m_dataStore->WithTransaction(
      [&] {
        auto validatedParent = m_dataStore->Validate(parent);
        auto children = m_dataStore->LoadChildren(validatedParent);
        auto childIterator = std::find_if(children.begin(), children.end(),
          [&] (auto& child) {
            return child.m_name == name;
          });
        if(childIterator == children.end()) {
          RegistryEntry validatedEntry(RegistryEntry::Type::VALUE, -1, name, 0);
          newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
          m_dataStore->Store(newEntry, value);
        } else {
          newEntry = m_dataStore->Store(*childIterator, value);
        }
      });
    return newEntry;
  }

  template<typename ContainerType, typename RegistryDataStoreType>
  void RegistryServlet<ContainerType, RegistryDataStoreType>::OnDeleteRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry) {
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        m_dataStore->Delete(validatedEntry);
      });
  }
}
}

#endif
