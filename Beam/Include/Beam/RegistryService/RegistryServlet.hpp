#ifndef BEAM_REGISTRY_SERVLET_HPP
#define BEAM_REGISTRY_SERVLET_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/RegistryService/RegistryServices.hpp"
#include "Beam/RegistryService/RegistrySession.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"

namespace Beam::RegistryService {

  /**
   * Provides access to a registry of resources and data.
   * @param C The container instantiating this servlet.
   * @param D The type of data store to use.
   */
  template<typename C, typename D>
  class RegistryServlet {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;

      /** The type of RegistryDataStore used. */
      using RegistryDataStore = GetTryDereferenceType<D>;

      /**
       * Constructs a RegistryServlet.
       * @param dataStore The data store to use.
       */
      template<typename DF>
      RegistryServlet(DF&& dataStore);

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void Close();

    private:
      GetOptionalLocalPtr<D> m_dataStore;
      IO::OpenState m_openState;

      RegistryServlet(const RegistryServlet&) = delete;
      RegistryServlet& operator =(const RegistryServlet&) = delete;
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

  template<typename D>
  struct MetaRegistryServlet {
    static constexpr auto SupportsParallelism = true;
    using Session = RegistrySession;
    template<typename C>
    struct apply {
      using type = RegistryServlet<C, D>;
    };
  };

  template<typename C, typename D>
  template<typename DF>
  RegistryServlet<C, D>::RegistryServlet(DF&& dataStore)
    : m_dataStore(std::forward<DF>(dataStore)) {}

  template<typename C, typename D>
  void RegistryServlet<C, D>::RegisterServices(
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

  template<typename C, typename D>
  void RegistryServlet<C, D>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_dataStore->Close();
    m_openState.Close();
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnLoadPathRequest(
      ServiceProtocolClient& client, const RegistryEntry& root,
      const std::string& path) {
    auto entry = RegistryEntry();
    m_dataStore->WithTransaction(
      [&] {
        auto validatedRoot = m_dataStore->Validate(root);
        entry = LoadRegistryEntry(*m_dataStore, validatedRoot, path);
      });
    return entry;
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnLoadParentRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry) {
    auto parent = RegistryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(registryEntry);
      parent = m_dataStore->LoadParent(validatedEntry);
    });
    return parent;
  }

  template<typename C, typename D>
  std::vector<RegistryEntry> RegistryServlet<C, D>::OnLoadChildrenRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry) {
    auto children = std::vector<RegistryEntry>();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(registryEntry);
      children = m_dataStore->LoadChildren(validatedEntry);
    });
    return children;
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnMakeDirectoryRequest(
      ServiceProtocolClient& client, const std::string& name,
      const RegistryEntry& parent) {
    auto newEntry = RegistryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedParent = m_dataStore->Validate(parent);
      auto validatedEntry = RegistryEntry(RegistryEntry::Type::DIRECTORY, -1,
        name, 0);
      newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
    });
    return newEntry;
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnCopyRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry,
      const RegistryEntry& destination) {
    auto copiedEntry = RegistryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(registryEntry);
      auto validatedDestination = m_dataStore->Validate(destination);
      copiedEntry = m_dataStore->Copy(validatedEntry, validatedDestination);
    });
    return copiedEntry;
  }

  template<typename C, typename D>
  void RegistryServlet<C, D>::OnMoveRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        auto validatedDestination = m_dataStore->Validate(destination);
        m_dataStore->Move(validatedEntry, validatedDestination);
      });
  }

  template<typename C, typename D>
  IO::SharedBuffer RegistryServlet<C, D>::OnLoadValueRequest(
      ServiceProtocolClient& client, const RegistryEntry& registryEntry) {
    auto value = IO::SharedBuffer();
    m_dataStore->WithTransaction(
      [&] {
        auto validatedEntry = m_dataStore->Validate(registryEntry);
        value = m_dataStore->Load(validatedEntry);
      });
    return value;
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnMakeValueRequest(
      ServiceProtocolClient& client, const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto newEntry = RegistryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedParent = m_dataStore->Validate(parent);
      auto validatedEntry = RegistryEntry(RegistryEntry::Type::VALUE, -1, name,
        0);
      newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
      m_dataStore->Store(newEntry, value);
    });
    return newEntry;
  }

  template<typename C, typename D>
  RegistryEntry RegistryServlet<C, D>::OnStoreValueRequest(
      ServiceProtocolClient& client, const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto newEntry = RegistryEntry();
    m_dataStore->WithTransaction([&] {
      auto validatedParent = m_dataStore->Validate(parent);
      auto children = m_dataStore->LoadChildren(validatedParent);
      auto childIterator = std::find_if(children.begin(), children.end(),
        [&] (auto& child) {
          return child.m_name == name;
        });
      if(childIterator == children.end()) {
        auto validatedEntry = RegistryEntry(RegistryEntry::Type::VALUE, -1,
          name, 0);
        newEntry = m_dataStore->Copy(validatedEntry, validatedParent);
        m_dataStore->Store(newEntry, value);
      } else {
        newEntry = m_dataStore->Store(*childIterator, value);
      }
    });
    return newEntry;
  }

  template<typename C, typename D>
  void RegistryServlet<C, D>::OnDeleteRequest(ServiceProtocolClient& client,
      const RegistryEntry& registryEntry) {
    m_dataStore->WithTransaction([&] {
      auto validatedEntry = m_dataStore->Validate(registryEntry);
      m_dataStore->Delete(validatedEntry);
    });
  }
}

#endif
