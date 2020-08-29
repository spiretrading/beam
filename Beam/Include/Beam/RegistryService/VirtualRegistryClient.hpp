#ifndef BEAM_VIRTUALREGISTRYCLIENT_HPP
#define BEAM_VIRTUALREGISTRYCLIENT_HPP
#include <memory>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class VirtualRegistryClient
      \brief Provides a pure virtual interface to a RegistryClient.
   */
  class VirtualRegistryClient : private boost::noncopyable {
    public:
      virtual ~VirtualRegistryClient();

      virtual RegistryEntry LoadPath(const RegistryEntry& root,
        const std::string& path) = 0;

      virtual RegistryEntry LoadParent(const RegistryEntry& registryEntry) = 0;

      virtual std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& registryEntry) = 0;

      virtual RegistryEntry MakeDirectory(const std::string& name,
        const RegistryEntry& parent) = 0;

      virtual RegistryEntry Copy(const RegistryEntry& registryEntry,
        const RegistryEntry& destination) = 0;

      virtual void Move(const RegistryEntry& registryEntry,
        const RegistryEntry& destination) = 0;

      virtual IO::SharedBuffer Load(const RegistryEntry& registryEntry) = 0;

      template<typename T>
      void Load(const RegistryEntry& registryEntry, Out<T> value);

      virtual RegistryEntry MakeValue(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent) = 0;

      template<typename T>
      RegistryEntry MakeValue(const std::string& name, const T& value,
        const RegistryEntry& parent);

      virtual RegistryEntry Store(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent) = 0;

      template<typename T>
      RegistryEntry Store(const std::string& name, const T& value,
        const RegistryEntry& parent);

      virtual void Delete(const RegistryEntry& registryEntry) = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualRegistryClient.
      VirtualRegistryClient();
  };

  /*! \class WrapperRegistryClient
      \brief Wraps a RegistryClient providing it with a virtual interface.
      \tparam ClientType The type of RegistryClient to wrap.
   */
  template<typename ClientType>
  class WrapperRegistryClient : public VirtualRegistryClient {
    public:

      //! The RegistryClient to wrap.
      using Client = typename TryDereferenceType<ClientType>::type;

      //! Constructs a WrapperRegistryClient.
      /*!
        \param client The RegistryClient to wrap.
      */
      template<typename RegistryClientForward>
      WrapperRegistryClient(RegistryClientForward&& client);

      virtual ~WrapperRegistryClient();

      virtual RegistryEntry LoadPath(const RegistryEntry& root,
        const std::string& path);

      virtual RegistryEntry LoadParent(const RegistryEntry& registryEntry);

      virtual std::vector<RegistryEntry> LoadChildren(
        const RegistryEntry& registryEntry);

      virtual RegistryEntry MakeDirectory(const std::string& name,
        const RegistryEntry& parent);

      virtual RegistryEntry Copy(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      virtual void Move(const RegistryEntry& registryEntry,
        const RegistryEntry& destination);

      virtual IO::SharedBuffer Load(const RegistryEntry& registryEntry);

      virtual RegistryEntry MakeValue(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      virtual RegistryEntry Store(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      virtual void Delete(const RegistryEntry& registryEntry);

      virtual void Close();

    private:
      typename OptionalLocalPtr<ClientType>::type m_client;
  };

  //! Wraps a RegistryClient into a VirtualRegistryClient.
  /*!
    \param client The client to wrap.
  */
  template<typename RegistryClient>
  std::unique_ptr<VirtualRegistryClient> MakeVirtualRegistryClient(
      RegistryClient&& client) {
    return std::make_unique<WrapperRegistryClient<RegistryClient>>(
      std::forward<RegistryClient>(client));
  }

  //! Loads a directory, or creates it if it doesn't already exist.
  /*!
    \param registryClient The RegistryClient to use.
    \param path The path to the directory to load or create.
    \param parent The directory's parent.
    \return directory The directory that was loaded.
  */
  template<typename RegistryClient>
  RegistryEntry LoadOrCreateDirectory(RegistryClient& client,
      const std::string& path, const RegistryEntry& parent) {
    if(path.empty()) {
      return parent;
    }
    std::string::size_type delimiter = path.find('/');
    std::string segment;
    if(delimiter == std::string::npos) {
      segment = path;
    } else {
      segment = path.substr(0, delimiter);
    }
    RegistryEntry entry;
    try {
      entry = client.LoadPath(parent, segment);
    } catch(const Services::ServiceRequestException&) {
      entry = client.MakeDirectory(segment, parent);
    }
    if(delimiter == std::string::npos) {
      return entry;
    }
    return LoadOrCreateDirectory(client, path.substr(delimiter + 1), entry);
  }

  inline VirtualRegistryClient::~VirtualRegistryClient() {}

  template<typename T>
  void VirtualRegistryClient::Load(const RegistryEntry& registryEntry,
      Out<T> value) {
    IO::SharedBuffer buffer = Load(registryEntry);
    Serialization::BinaryReceiver<IO::SharedBuffer> receiver;
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(*value);
  }

  template<typename T>
  RegistryEntry VirtualRegistryClient::MakeValue(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    IO::SharedBuffer buffer;
    Serialization::BinarySender<IO::SharedBuffer> sender;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return MakeValue(name, buffer, parent);
  }

  template<typename T>
  RegistryEntry VirtualRegistryClient::Store(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    IO::SharedBuffer buffer;
    Serialization::BinarySender<IO::SharedBuffer> sender;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return Store(name, buffer, parent);
  }

  inline VirtualRegistryClient::VirtualRegistryClient() {}

  template<typename ClientType>
  template<typename RegistryClientForward>
  WrapperRegistryClient<ClientType>::WrapperRegistryClient(
      RegistryClientForward&& client)
      : m_client(std::forward<RegistryClientForward>(client)) {}

  template<typename ClientType>
  WrapperRegistryClient<ClientType>::~WrapperRegistryClient() {}

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::LoadPath(
      const RegistryEntry& root, const std::string& path) {
    return m_client->LoadPath(root, path);
  }

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::LoadParent(
      const RegistryEntry& registryEntry) {
    return m_client->LoadParent(registryEntry);
  }

  template<typename ClientType>
  std::vector<RegistryEntry> WrapperRegistryClient<ClientType>::LoadChildren(
      const RegistryEntry& registryEntry) {
    return m_client->LoadChildren(registryEntry);
  }

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::MakeDirectory(
      const std::string& name, const RegistryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::Copy(
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    return m_client->Copy(registryEntry, destination);
  }

  template<typename ClientType>
  void WrapperRegistryClient<ClientType>::Move(
      const RegistryEntry& registryEntry, const RegistryEntry& destination) {
    m_client->Move(registryEntry, destination);
  }

  template<typename ClientType>
  IO::SharedBuffer WrapperRegistryClient<ClientType>::Load(
      const RegistryEntry& registryEntry) {
    return m_client->Load(registryEntry);
  }

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::MakeValue(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    return m_client->MakeValue(name, value, parent);
  }

  template<typename ClientType>
  RegistryEntry WrapperRegistryClient<ClientType>::Store(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    return m_client->Store(name, value, parent);
  }

  template<typename ClientType>
  void WrapperRegistryClient<ClientType>::Delete(
      const RegistryEntry& registryEntry) {
    m_client->Delete(registryEntry);
  }

  template<typename ClientType>
  void WrapperRegistryClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
