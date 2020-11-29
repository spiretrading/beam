#ifndef BEAM_TO_PYTHON_REGISTRY_CLIENT_HPP
#define BEAM_TO_PYTHON_REGISTRY_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/RegistryService/RegistryClientBox.hpp"

namespace Beam::RegistryService {

  /**
   * Wraps a RegistryClient for use with Python.
   * @param <C> The type of client to wrap.
   */
  template<typename C>
  class ToPythonRegistryClient {
    public:

      /** The type of client to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonRegistryClient.
       * @param args The arguments to forward to the Client's constructor.
       */
      template<typename... Args>
      ToPythonRegistryClient(Args&&... args);

      ToPythonRegistryClient(ToPythonRegistryClient&&) = default;

      ~ToPythonRegistryClient();

      /** Returns the wrapped client. */
      const Client& GetClient() const;

      /** Returns the wrapped client. */
      Client& GetClient();

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

      ToPythonRegistryClient& operator =(ToPythonRegistryClient&&) = default;

    private:
      boost::optional<Client> m_client;

      ToPythonRegistryClient(const ToPythonRegistryClient&) = delete;
      ToPythonRegistryClient& operator =(
        const ToPythonRegistryClient&) = delete;
  };

  template<typename Client>
  ToPythonRegistryClient(Client&&) ->
    ToPythonRegistryClient<std::decay_t<Client>>;

  template<typename C>
  template<typename... Args>
  ToPythonRegistryClient<C>::ToPythonRegistryClient(Args&&... args)
    : m_client((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonRegistryClient<C>::~ToPythonRegistryClient() {
    auto release = Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonRegistryClient<C>::Client&
      ToPythonRegistryClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonRegistryClient<C>::Client&
      ToPythonRegistryClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::LoadPath(const RegistryEntry& root,
      const std::string& path) {
    auto release = Python::GilRelease();
    return m_client->LoadPath(root, path);
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::LoadParent(
      const RegistryEntry& entry) {
    auto release = Python::GilRelease();
    return m_client->LoadParent(entry);
  }

  template<typename C>
  std::vector<RegistryEntry> ToPythonRegistryClient<C>::LoadChildren(
      const RegistryEntry& entry) {
    auto release = Python::GilRelease();
    return m_client->LoadChildren(entry);
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::MakeDirectory(
      const std::string& name, const RegistryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->MakeDirectory(name, parent);
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::Copy(const RegistryEntry& entry,
      const RegistryEntry& destination) {
    auto release = Python::GilRelease();
    return m_client->Copy(entry, destination);
  }

  template<typename C>
  void ToPythonRegistryClient<C>::Move(const RegistryEntry& entry,
      const RegistryEntry& destination) {
    auto release = Python::GilRelease();
    m_client->Move(entry, destination);
  }

  template<typename C>
  IO::SharedBuffer ToPythonRegistryClient<C>::Load(const RegistryEntry& entry) {
    auto release = Python::GilRelease();
    return m_client->Load(entry);
  }

  template<typename C>
  template<typename T>
  void ToPythonRegistryClient<C>::Load(const RegistryEntry& entry,
      Out<T> value) {
    auto release = Python::GilRelease();
    m_client->Load(entry, Store(value));
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::MakeValue(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->MakeValue(name, value, parent);
  }

  template<typename C>
  template<typename T>
  RegistryEntry ToPythonRegistryClient<C>::MakeValue(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->MakeValue(name, value, parent);
  }

  template<typename C>
  RegistryEntry ToPythonRegistryClient<C>::Store(const std::string& name,
      const IO::SharedBuffer& value, const RegistryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->Store(name, value, parent);
  }

  template<typename C>
  template<typename T>
  RegistryEntry ToPythonRegistryClient<C>::Store(const std::string& name,
      const T& value, const RegistryEntry& parent) {
    auto release = Python::GilRelease();
    return m_client->Store(name, value, parent);
  }

  template<typename C>
  void ToPythonRegistryClient<C>::Delete(const RegistryEntry& entry) {
    auto release = Python::GilRelease();
    m_client->Delete(entry);
  }

  template<typename C>
  void ToPythonRegistryClient<C>::Close() {
    auto release = Python::GilRelease();
    m_client->Close();
  }
}

#endif
