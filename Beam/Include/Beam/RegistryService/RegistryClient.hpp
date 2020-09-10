#ifndef BEAM_REGISTRY_CLIENT_HPP
#define BEAM_REGISTRY_CLIENT_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/RegistryService/RegistryServices.hpp"

namespace Beam::RegistryService {

  /**
   * Client used to access the registry.
   * @param B The type used to build ServiceProtocolClients to the server.
   */
  template<typename B>
  class RegistryClient {
    public:

      /** The type used to build ServiceProtocolClients to the server. */
      using ServiceProtocolClientBuilder = GetTryDereferenceType<B>;

      /**
       * Constructs a RegistryClient.
       * @param clientBuilder Initializes the ServiceProtocolClientBuilder.
       */
      template<typename BF>
      explicit RegistryClient(BF&& clientBuilder);

      ~RegistryClient();

      /**
       * Loads a RegistryEntry from a path.
       * @param root The RegistryEntry to load from.
       * @param path The path from the <i>root</i> to the RegistryEntry to load.
       * @return The RegistryEntry at the specified path from the <i>root</i>.
       */
      RegistryEntry LoadPath(const RegistryEntry& root,
        const std::string& path);

      /**
       * Loads a RegistryEntry's parent.
       * @param registryEntry The RegistryEntry whose parent is to be loaded.
       * @return The <i>registryEntry</i>'s parent.
       */
      RegistryEntry LoadParent(const RegistryEntry& registryEntry);

      /**
       * Loads a RegistryEntry's children.
       * @param registryEntry The RegistryEntry whose children are to be loaded.
       * @return The <i>registryEntry</i>'s children.
       */
      std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& registryEntry);

      /**
       * Creates a RegistryEntry directory.
       * @param name The name of the directory to make.
       * @param parent The directory's parent.
       * @return The RegistryEntry representing the new directory.
       */
      RegistryEntry MakeDirectory(const std::string& name,
        const RegistryEntry& parent);

      /**
       * Copies a RegistryEntry.
       * @param registryEntry The RegistryEntry to copy.
       * @param destination Where to copy the RegistryEntry to.
       * @return The copy made of the RegistryEntry.
       */
      RegistryEntry Copy(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      /**
       * Moves a RegistryEntry.
       * @param registryEntry The RegistryEntry to copy.
       * @param destination Where to move the RegistryEntry to.
       */
      void Move(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      /**
       * Loads a registry value.
       * @param registryEntry The RegistryEntry of the value to load.
       * @return The value associated with the <i>registryEntry</i>.
       */
      IO::SharedBuffer Load(const RegistryEntry& registryEntry);

      /**
       * Loads a registry value.
       * @param registryEntry The RegistryEntry of the value to load.
       * @param value Where to store the loaded value.
       */
      template<typename T>
      void Load(const RegistryEntry& registryEntry, Out<T> value);

      /**
       * Creates a RegistryEntry value.
       * @param name The name of the registry value.
       * @param value The value to store.
       * @param parent The directory's parent.
       * @return The RegistryEntry representing the new directory.
       */
      RegistryEntry MakeValue(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      /**
       * Creates a RegistryEntry value.
       * @param name The name of the registry value.
       * @param value The value to store.
       * @param parent The directory's parent.
       * @return The RegistryEntry representing the value.
       */
      template<typename T>
      RegistryEntry MakeValue(const std::string& name, const T& value,
        const RegistryEntry& parent);

      /**
       * Creates or updates a RegistryEntry value.
       * @param name The name of the registry value.
       * @param value The value to store.
       * @param parent The directory's parent.
       * @return The RegistryEntry representing the value.
       */
      RegistryEntry Store(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      /**
       * Creates or updates a RegistryEntry value.
       * @param name The name of the registry value.
       * @param value The value to store.
       * @param parent The directory's parent.
       * @return The RegistryEntry representing the value.
       */
      template<typename T>
      RegistryEntry Store(const std::string& name, const T& value,
        const RegistryEntry& parent);

      /**
       * Deletes a RegistryEntry.
       * @param registryEntry The RegistryEntry to delete.
       */
      void Delete(const RegistryEntry& registryEntry);

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<B> m_clientHandler;
      IO::OpenState m_openState;

      RegistryClient(const RegistryClient&) = delete;
      RegistryClient& operator =(const RegistryClient&) = delete;
  };

  template<typename B>
  template<typename BF>
  RegistryClient<B>::RegistryClient(BF&& clientBuilder)
      : m_clientHandler(std::forward<BF>(clientBuilder)) {
    RegisterRegistryServices(Beam::Store(m_clientHandler.GetSlots()));
  }

  template<typename B>
  RegistryClient<B>::~RegistryClient() {
    Close();
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::LoadPath(const RegistryEntry& root,
      const std::string& path) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadPathService>(root, path);
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::LoadParent(
      const RegistryEntry& registryEntry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadParentService>(registryEntry);
  }

  template<typename B>
  std::vector<RegistryEntry> RegistryClient<B>::LoadChildren(
      const RegistryEntry& registryEntry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadChildrenService>(registryEntry);
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::MakeDirectory(const std::string& name,
      const RegistryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeDirectoryService>(name, parent);
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::Copy(const RegistryEntry& registryEntry,
      const RegistryEntry& destination) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<CopyService>(registryEntry,
      destination);
  }

  template<typename B>
  void RegistryClient<B>::Move(const RegistryEntry& registryEntry,
      const RegistryEntry& destination) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<MoveService>(registryEntry, destination);
  }

  template<typename B>
  IO::SharedBuffer RegistryClient<B>::Load(const RegistryEntry& registryEntry) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadValueService>(registryEntry);
  }

  template<typename B>
  template<typename T>
  void RegistryClient<B>::Load(const RegistryEntry& registryEntry,
      Out<T> value) {
    auto buffer = Load(registryEntry);
    auto receiver = Serialization::BinaryReceiver<IO::SharedBuffer>();
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(*value);
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::MakeValue(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeValueService>(name, value, parent);
  }

  template<typename B>
  template<typename T>
  RegistryEntry RegistryClient<B>::MakeValue(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto buffer = IO::SharedBuffer();
    auto sender = Serialization::BinarySender<IO::SharedBuffer>();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return MakeValue(name, buffer, parent);
  }

  template<typename B>
  RegistryEntry RegistryClient<B>::Store(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto client = m_clientHandler.GetClient();
    return client->template SendRequest<StoreValueService>(name, value, parent);
  }

  template<typename B>
  template<typename T>
  RegistryEntry RegistryClient<B>::Store(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto buffer = IO::SharedBuffer();
    auto sender = Serialization::BinarySender<IO::SharedBuffer>();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return Store(name, buffer, parent);
  }

  template<typename B>
  void RegistryClient<B>::Delete(const RegistryEntry& registryEntry) {
    auto client = m_clientHandler.GetClient();
    client->template SendRequest<DeleteService>(registryEntry);
  }

  template<typename B>
  void RegistryClient<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_clientHandler.Close();
    m_openState.Close();
  }
}

#endif
