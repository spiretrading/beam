#ifndef BEAM_REGISTRY_CLIENT_BOX_HPP
#define BEAM_REGISTRY_CLIENT_BOX_HPP
#include <type_traits>
#include <utility>
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam::RegistryService {

  /** Provides a generic interface to an arbitrary RegistryClient. */
  class RegistryClientBox {
    public:

      /**
       * Constructs a RegistryClientBox of a specified type using emplacement.
       * @param <T> The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<typename T, typename... Args>
      explicit RegistryClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a RegistryClientBox by copying an existing client.
       * @param client The client to copy.
       */
      template<typename RegistryClient>
      explicit RegistryClientBox(RegistryClient client);

      explicit RegistryClientBox(RegistryClientBox* client);

      explicit RegistryClientBox(
        const std::shared_ptr<RegistryClientBox>& client);

      explicit RegistryClientBox(
        const std::unique_ptr<RegistryClientBox>& client);

      RegistryEntry LoadPath(const RegistryEntry& root,
        const std::string& path);

      RegistryEntry LoadParent(const RegistryEntry& entry);

      std::vector<RegistryEntry> LoadChildren(const RegistryEntry& entry);

      RegistryEntry MakeDirectory(const std::string& name,
        const RegistryEntry& parent);

      RegistryEntry Copy(const RegistryEntry& entry,
        const RegistryEntry& destination);

      void Move(const RegistryEntry& entry, const RegistryEntry& destination);

      IO::SharedBuffer Load(const RegistryEntry& entry);

      template<typename T>
      void Load(const RegistryEntry& entry, Out<T> value);

      RegistryEntry MakeValue(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      template<typename T>
      RegistryEntry MakeValue(const std::string& name, const T& value,
        const RegistryEntry& parent);

      RegistryEntry Store(const std::string& name,
        const IO::SharedBuffer& value, const RegistryEntry& parent);

      template<typename T>
      RegistryEntry Store(const std::string& name, const T& value,
        const RegistryEntry& parent);

      void Delete(const RegistryEntry& entry);

      void Close();

    private:
      struct VirtualRegistryClient {
        virtual ~VirtualRegistryClient() = default;
        virtual RegistryEntry LoadPath(const RegistryEntry& root,
          const std::string& path) = 0;
        virtual RegistryEntry LoadParent(const RegistryEntry& entry) = 0;
        virtual std::vector<RegistryEntry> LoadChildren(
          const RegistryEntry& entry) = 0;
        virtual RegistryEntry MakeDirectory(const std::string& name,
          const RegistryEntry& parent) = 0;
        virtual RegistryEntry Copy(const RegistryEntry& entry,
          const RegistryEntry& destination) = 0;
        virtual void Move(const RegistryEntry& entry,
          const RegistryEntry& destination) = 0;
        virtual IO::SharedBuffer Load(const RegistryEntry& entry) = 0;
        virtual RegistryEntry MakeValue(const std::string& name,
          const IO::SharedBuffer& value, const RegistryEntry& parent) = 0;
        virtual RegistryEntry Store(const std::string& name,
          const IO::SharedBuffer& value, const RegistryEntry& parent) = 0;
        virtual void Delete(const RegistryEntry& entry) = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedRegistryClient final : VirtualRegistryClient {
        using Client = C;
        GetOptionalLocalPtr<Client> m_client;

        template<typename... Args>
        WrappedRegistryClient(Args&&... args);
        RegistryEntry LoadPath(const RegistryEntry& root,
          const std::string& path) override;
        RegistryEntry LoadParent(const RegistryEntry& entry) override;
        std::vector<RegistryEntry> LoadChildren(
          const RegistryEntry& entry) override;
        RegistryEntry MakeDirectory(const std::string& name,
          const RegistryEntry& parent) override;
        RegistryEntry Copy(const RegistryEntry& entry,
          const RegistryEntry& destination) override;
        void Move(const RegistryEntry& entry,
          const RegistryEntry& destination) override;
        IO::SharedBuffer Load(const RegistryEntry& entry) override;
        RegistryEntry MakeValue(const std::string& name,
          const IO::SharedBuffer& value, const RegistryEntry& parent) override;
        RegistryEntry Store(const std::string& name,
          const IO::SharedBuffer& value, const RegistryEntry& parent) override;
        void Delete(const RegistryEntry& entry) override;
        void Close() override;
      };
      std::shared_ptr<VirtualRegistryClient> m_client;
  };

  /**
   * Loads a directory, or creates it if it doesn't already exist.
   * @param registryClient The RegistryClient to use.
   * @param path The path to the directory to load or create.
   * @param parent The directory's parent.
   * @return directory The directory that was loaded.
   */
  template<typename RegistryClient>
  RegistryEntry LoadOrCreateDirectory(RegistryClient& client,
      const std::string& path, const RegistryEntry& parent) {
    if(path.empty()) {
      return parent;
    }
    auto delimiter = path.find('/');
    auto segment = [&] {
      if(delimiter == std::string::npos) {
        return path;
      } else {
        return path.substr(0, delimiter);
      }
    }();
    auto entry = [&] {
      try {
        return client.LoadPath(parent, segment);
      } catch(const Services::ServiceRequestException&) {
        return client.MakeDirectory(segment, parent);
      }
    }();
    if(delimiter == std::string::npos) {
      return entry;
    }
    return LoadOrCreateDirectory(client, path.substr(delimiter + 1), entry);
  }

  template<typename T, typename... Args>
  RegistryClientBox::RegistryClientBox(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedRegistryClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename RegistryClient>
  RegistryClientBox::RegistryClientBox(RegistryClient client)
    : RegistryClientBox(std::in_place_type<RegistryClient>,
        std::move(client)) {}

  inline RegistryClientBox::RegistryClientBox(RegistryClientBox* client)
    : RegistryClientBox(*client) {}

  inline RegistryClientBox::RegistryClientBox(
    const std::shared_ptr<RegistryClientBox>& client)
    : RegistryClientBox(*client) {}

  inline RegistryClientBox::RegistryClientBox(
    const std::unique_ptr<RegistryClientBox>& client)
    : RegistryClientBox(*client) {}

  inline RegistryEntry RegistryClientBox::LoadPath(const RegistryEntry& root,
      const std::string& path) {
    return m_client->LoadPath(root, path);
  }

  inline RegistryEntry RegistryClientBox::LoadParent(
      const RegistryEntry& entry) {
    return m_client->LoadParent(entry);
  }

  inline std::vector<RegistryEntry> RegistryClientBox::LoadChildren(
      const RegistryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  inline RegistryEntry RegistryClientBox::MakeDirectory(const std::string& name,
      const RegistryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  inline RegistryEntry RegistryClientBox::Copy(const RegistryEntry& entry,
      const RegistryEntry& destination) {
    return m_client->Copy(entry, destination);
  }

  inline void RegistryClientBox::Move(const RegistryEntry& entry,
      const RegistryEntry& destination) {
    m_client->Move(entry, destination);
  }

  inline IO::SharedBuffer RegistryClientBox::Load(const RegistryEntry& entry) {
    return m_client->Load(entry);
  }

  template<typename T>
  void RegistryClientBox::Load(const RegistryEntry& entry, Out<T> value) {
    auto buffer = Load(entry);
    auto receiver = Serialization::BinaryReceiver<IO::SharedBuffer>();
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(*value);
  }

  inline RegistryEntry RegistryClientBox::MakeValue(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    return m_client->MakeValue(name, value, parent);
  }

  template<typename T>
  RegistryEntry RegistryClientBox::MakeValue(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto buffer = IO::SharedBuffer();
    auto sender = Serialization::BinarySender<IO::SharedBuffer>();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return MakeValue(name, buffer, parent);
  }

  inline RegistryEntry RegistryClientBox::Store(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    return m_client->Store(name, value, parent);
  }

  template<typename T>
  RegistryEntry RegistryClientBox::Store(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto buffer = IO::SharedBuffer();
    auto sender = Serialization::BinarySender<IO::SharedBuffer>();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    return Store(name, buffer, parent);
  }

  inline void RegistryClientBox::Delete(const RegistryEntry& entry) {
    m_client->Delete(entry);
  }

  inline void RegistryClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  RegistryClientBox::WrappedRegistryClient<C>::WrappedRegistryClient(
    Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::LoadPath(
      const RegistryEntry& root, const std::string& path) {
    return m_client->LoadPath(root, path);
  }

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::LoadParent(
      const RegistryEntry& entry) {
    return m_client->LoadParent(entry);
  }

  template<typename C>
  std::vector<RegistryEntry>
      RegistryClientBox::WrappedRegistryClient<C>::LoadChildren(
      const RegistryEntry& entry) {
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::MakeDirectory(
      const std::string& name, const RegistryEntry& parent) {
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::Copy(
      const RegistryEntry& entry, const RegistryEntry& destination) {
    return m_client->Copy(entry, destination);
  }

  template<typename C>
  void RegistryClientBox::WrappedRegistryClient<C>::Move(
      const RegistryEntry& entry, const RegistryEntry& destination) {
    m_client->Move(entry, destination);
  }

  template<typename C>
  IO::SharedBuffer RegistryClientBox::WrappedRegistryClient<C>::Load(
      const RegistryEntry& entry) {
    return m_client->Load(entry);
  }

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::MakeValue(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    return m_client->MakeValue(name, value, parent);
  }

  template<typename C>
  RegistryEntry RegistryClientBox::WrappedRegistryClient<C>::Store(
      const std::string& name, const IO::SharedBuffer& value,
      const RegistryEntry& parent) {
    return m_client->Store(name, value, parent);
  }

  template<typename C>
  void RegistryClientBox::WrappedRegistryClient<C>::Delete(
      const RegistryEntry& entry) {
    m_client->Delete(entry);
  }

  template<typename C>
  void RegistryClientBox::WrappedRegistryClient<C>::Close() {
    m_client->Close();
  }
}

#endif
