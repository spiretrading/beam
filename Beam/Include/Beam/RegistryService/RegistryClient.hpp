#ifndef BEAM_REGISTRYCLIENT_HPP
#define BEAM_REGISTRYCLIENT_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClientHandler.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/RegistryService/RegistryServices.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class RegistryClient
      \brief Client used to access the registry.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients to the server.
   */
  template<typename ServiceProtocolClientBuilderType>
  class RegistryClient : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients to the server.
      using ServiceProtocolClientBuilder =
        typename TryDereferenceType<ServiceProtocolClientBuilderType>::type;

      //! Constructs a RegistryClient.
      /*!
        \param clientBuilder Initializes the ServiceProtocolClientBuilder.
      */
      template<typename ClientBuilderForward>
      RegistryClient(ClientBuilderForward&& clientBuilder);

      ~RegistryClient();

      //! Loads a RegistryEntry from a path.
      /*!
        \param root The RegistryEntry to load from.
        \param path The path from the <i>root</i> to the RegistryEntry to load.
        \return The RegistryEntry at the specified path from the <i>root</i>.
      */
      RegistryEntry LoadPath(const RegistryEntry& root,
        const std::string& path);

      //! Loads a RegistryEntry's parent.
      /*!
        \param registryEntry The RegistryEntry whose parent is to be loaded.
        \return The <i>registryEntry</i>'s parent.
      */
      RegistryEntry LoadParent(const RegistryEntry& registryEntry);

      //! Loads a RegistryEntry's children.
      /*!
        \param registryEntry The RegistryEntry whose children are to be loaded.
        \return The <i>registryEntry</i>'s children.
      */
      std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& registryEntry);

      //! Creates a RegistryEntry directory.
      /*!
        \param name The name of the directory to make.
        \param parent The directory's parent.
        \return The RegistryEntry representing the new directory.
      */
      RegistryEntry MakeDirectory(const std::string& name,
        const RegistryEntry& parent);

      //! Copies a RegistryEntry.
      /*!
        \param registryEntry The RegistryEntry to copy.
        \param destination Where to copy the RegistryEntry to.
        \return The copy made of the RegistryEntry.
      */
      RegistryEntry Copy(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      //! Moves a RegistryEntry.
      /*!
        \param registryEntry The RegistryEntry to copy.
        \param destination Where to move the RegistryEntry to.
      */
      void Move(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      //! Loads a registry value.
      /*!
        \param registryEntry The RegistryEntry of the value to load.
        \return The value associated with the <i>registryEntry</i>.
      */
      IO::SharedBuffer Load(const RegistryEntry& registryEntry);

      //! Loads a registry value.
      /*!
        \param registryEntry The RegistryEntry of the value to load.
        \param value Where to store the loaded value.
      */
      template<typename T>
      void Load(const RegistryEntry& registryEntry, Out<T> value);

      //! Creates a RegistryEntry value.
      /*!
        \param name The name of the registry value.
        \param value The value to store.
        \param parent The directory's parent.
        \return The RegistryEntry representing the new directory.
      */
      RegistryEntry MakeValue(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      //! Creates a RegistryEntry value.
      /*!
        \param name The name of the registry value.
        \param value The value to store.
        \param parent The directory's parent.
        \return The RegistryEntry representing the value.
      */
      template<typename T>
      RegistryEntry MakeValue(const std::string& name, const T& value,
        const RegistryEntry& parent);

      //! Creates or updates a RegistryEntry value.
      /*!
        \param name The name of the registry value.
        \param value The value to store.
        \param parent The directory's parent.
        \return The RegistryEntry representing the value.
      */
      RegistryEntry Store(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      //! Creates or updates a RegistryEntry value.
      /*!
        \param name The name of the registry value.
        \param value The value to store.
        \param parent The directory's parent.
        \return The RegistryEntry representing the value.
      */
      template<typename T>
      RegistryEntry Store(const std::string& name, const T& value,
        const RegistryEntry& parent);

      //! Deletes a RegistryEntry.
      /*!
        \param registryEntry The RegistryEntry to delete.
      */
      void Delete(const RegistryEntry& registryEntry);

      void Close();

    private:
      using ServiceProtocolClient =
        typename ServiceProtocolClientBuilder::Client;
      Beam::Services::ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType> m_clientHandler;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename ClientBuilderForward>
  RegistryClient<ServiceProtocolClientBuilderType>::RegistryClient(
      ClientBuilderForward&& clientBuilder)
      : m_clientHandler(std::forward<ClientBuilderForward>(clientBuilder)) {
    RegisterRegistryServices(Beam::Store(m_clientHandler.GetSlots()));
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryClient<ServiceProtocolClientBuilderType>::~RegistryClient() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::LoadPath(
      const RegistryEntry& root, const std::string& path) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadPathService>(root, path);
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::LoadParent(
      const RegistryEntry& registryEntry) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadParentService>(registryEntry);
  }

  template<typename ServiceProtocolClientBuilderType>
  std::vector<RegistryEntry> RegistryClient<ServiceProtocolClientBuilderType>::
      LoadChildren(const RegistryEntry& registryEntry) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadChildrenService>(registryEntry);
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::
      MakeDirectory(const std::string& name, const RegistryEntry& parent) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeDirectoryService>(name, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::Copy(
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<CopyService>(registryEntry,
      destination);
  }

  template<typename ServiceProtocolClientBuilderType>
  void RegistryClient<ServiceProtocolClientBuilderType>::Move(
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    client->template SendRequest<MoveService>(registryEntry, destination);
  }

  template<typename ServiceProtocolClientBuilderType>
  IO::SharedBuffer RegistryClient<ServiceProtocolClientBuilderType>::Load(
      const RegistryEntry& registryEntry) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<LoadValueService>(registryEntry);
  }

  template<typename ServiceProtocolClientBuilderType>
  template<typename T>
  void RegistryClient<ServiceProtocolClientBuilderType>::Load(
      const RegistryEntry& registryEntry, Out<T> value) {
    IO::SharedBuffer buffer = Load(registryEntry);
    Serialization::BinaryReceiver<IO::SharedBuffer> receiver;
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(*value);
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::MakeValue(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<MakeValueService>(name, value, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  template<typename T>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::MakeValue(
      const std::string& name, const T& value, const RegistryEntry& parent) {
    IO::SharedBuffer buffer;
    Serialization::BinarySender<IO::SharedBuffer> sender;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return MakeValue(name, buffer, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::Store(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    return client->template SendRequest<StoreValueService>(name, value, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  template<typename T>
  RegistryEntry RegistryClient<ServiceProtocolClientBuilderType>::Store(
      const std::string& name, const T& value, const RegistryEntry& parent) {
    IO::SharedBuffer buffer;
    Serialization::BinarySender<IO::SharedBuffer> sender;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return Store(name, buffer, parent);
  }

  template<typename ServiceProtocolClientBuilderType>
  void RegistryClient<ServiceProtocolClientBuilderType>::Delete(
      const RegistryEntry& registryEntry) {
    std::shared_ptr<ServiceProtocolClient> client = m_clientHandler.GetClient();
    client->template SendRequest<DeleteService>(registryEntry);
  }

  template<typename ServiceProtocolClientBuilderType>
  void RegistryClient<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void RegistryClient<ServiceProtocolClientBuilderType>::Shutdown() {
    m_clientHandler.Close();
    m_openState.SetClosed();
  }
}
}

#endif
