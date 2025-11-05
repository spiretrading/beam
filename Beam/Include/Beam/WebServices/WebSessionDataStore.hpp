#ifndef BEAM_WEB_SESSION_DATA_STORE_HPP
#define BEAM_WEB_SESSION_DATA_STORE_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/Casts.hpp"
#include "Beam/Utilities/TypeTraits.hpp"
#include "Beam/WebServices/WebSession.hpp"

namespace Beam {

  /** Concept for types that implement the WebSessionDataStore interface. */
  template<typename T>
  concept IsWebSessionDataStore = IsConnection<T> && requires(T& t) {
    { t.template load<WebSession>(std::declval<const std::string&>()) } ->
      std::same_as<std::unique_ptr<WebSession>>;
    { t.store(std::declval<WebSession>()) };
    { t.remove(std::declval<WebSession>()) };
    { t.with_transaction(std::declval<std::function<void ()>>()) };
  };

  /** Base class used to store web session data. */
  class WebSessionDataStore {
    public:

      /**
       * Constructs a WebSessionDataStore of a specified type using emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsWebSessionDataStore T, typename... Args>
      explicit WebSessionDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a WebSessionDataStore by referencing an existing data store.
       * @param data_store The data store to reference.
       */
      template<DisableCopy<WebSessionDataStore> T> requires
        IsWebSessionDataStore<dereference_t<T>>
      WebSessionDataStore(T&& data_store);

      WebSessionDataStore(const WebSessionDataStore&) = default;
      WebSessionDataStore(WebSessionDataStore&&) = default;

      /**
       * Loads a session with a specified id.
       * @tparam S The type of session to load (must derive from WebSession).
       * @param id The session id.
       * @return The session with the specified id or nullptr if no such
       *         existing session exists.
       */
      template<std::derived_from<WebSession> S>
      std::unique_ptr<S> load(const std::string& id);

      /**
       * Stores a session.
       * @tparam S The type of session to store (must derive from WebSession).
       * @param session The session to store.
       */
      template<std::derived_from<WebSession> S>
      void store(const S& session);

      /**
       * Removes a session.
       * @tparam S The type of session to remove (must derive from WebSession).
       * @param session The session to remove.
       */
      template<std::derived_from<WebSession> S>
      void remove(const S& session);

      /**
       * Executes a transactional query.
       * @tparam F The type of transaction function.
       * @param transaction The transaction to perform.
       * @return The result of calling transaction().
       */
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);

      void close();

    private:
      struct VirtualDataStore {
        virtual ~VirtualDataStore() = default;

        virtual std::unique_ptr<WebSession> load(const std::string& id) = 0;
        virtual void store(const WebSession& session) = 0;
        virtual void remove(const WebSession& session) = 0;
        virtual void with_transaction(const std::function<void ()>&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedDataStore final : VirtualDataStore {
        using DataStore = D;
        local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedDataStore(Args&&... args);

        std::unique_ptr<WebSession> load(const std::string& id) override;
        void store(const WebSession& session) override;
        void remove(const WebSession& session) override;
        void with_transaction(
          const std::function<void ()>& transaction) override;
        void close() override;
      };
      VirtualPtr<VirtualDataStore> m_data_store;
  };

  template<IsWebSessionDataStore T, typename... Args>
  WebSessionDataStore::WebSessionDataStore(
    std::in_place_type_t<T>, Args&&... args)
    : m_data_store(
        make_virtual_ptr<WrappedDataStore<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<WebSessionDataStore> T> requires
    IsWebSessionDataStore<dereference_t<T>>
  WebSessionDataStore::WebSessionDataStore(T&& data_store)
    : m_data_store(make_virtual_ptr<WrappedDataStore<std::remove_cvref_t<T>>>(
        std::forward<T>(data_store))) {}

  template<std::derived_from<WebSession> S>
  std::unique_ptr<S> WebSessionDataStore::load(const std::string& id) {
    return static_pointer_cast<S>(m_data_store->load(id));
  }

  template<std::derived_from<WebSession> S>
  void WebSessionDataStore::store(const S& session) {
    m_data_store->store(session);
  }

  template<std::derived_from<WebSession> S>
  void WebSessionDataStore::remove(const S& session) {
    m_data_store->remove(session);
  }

  template<std::invocable<> F>
  decltype(auto) WebSessionDataStore::with_transaction(F&& transaction) {
    using R = std::invoke_result_t<F>;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_reference_t<R>*>(nullptr);
      m_data_store->with_transaction([&] {
        result = &(std::forward<F>(transaction)());
      });
      return *result;
    } else if constexpr(std::is_void_v<R>) {
      m_data_store->with_transaction(std::forward<F>(transaction));
    } else {
      auto result = boost::optional<R>();
      m_data_store->with_transaction([&] {
        result.emplace(std::forward<F>(transaction)());
      });
      return R(std::move(*result));
    }
  }

  inline void WebSessionDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  WebSessionDataStore::WrappedDataStore<D>::WrappedDataStore(Args&&... args)
      : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::unique_ptr<WebSession> WebSessionDataStore::WrappedDataStore<D>::load(
      const std::string& id) {
    return m_data_store->template load<WebSession>(id);
  }

  template<typename D>
  void WebSessionDataStore::WrappedDataStore<D>::store(
      const WebSession& session) {
    m_data_store->store(session);
  }

  template<typename D>
  void WebSessionDataStore::WrappedDataStore<D>::remove(
      const WebSession& session) {
    m_data_store->remove(session);
  }

  template<typename D>
  void WebSessionDataStore::WrappedDataStore<D>::with_transaction(
      const std::function<void ()>& transaction) {
    m_data_store->with_transaction(transaction);
  }

  template<typename D>
  void WebSessionDataStore::WrappedDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
