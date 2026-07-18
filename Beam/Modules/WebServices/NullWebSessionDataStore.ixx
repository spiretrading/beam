module;
#include "Prelude.hpp"

export module Beam:NullWebSessionDataStore;

import :WebSessionDataStore;

export namespace Beam {

  /** A data store used to avoid persisting sessions. */
  class NullWebSessionDataStore {
    public:

      /** Constructs a NullWebSessionDataStore. */
      NullWebSessionDataStore() = default;

      template<std::derived_from<WebSession> S>
      std::unique_ptr<S> load(const std::string& id);
      template<std::derived_from<WebSession> S>
      void store(const S& session);
      template<std::derived_from<WebSession> S>
      void remove(const S& session);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();
  };

  template<std::derived_from<WebSession> S>
  std::unique_ptr<S> NullWebSessionDataStore::load(const std::string& id) {
    return nullptr;
  }

  template<std::derived_from<WebSession> S>
  void NullWebSessionDataStore::store(const S& session) {}

  template<std::derived_from<WebSession> S>
  void NullWebSessionDataStore::remove(const S& session) {}

  template<std::invocable<> F>
  decltype(auto) NullWebSessionDataStore::with_transaction(F&& transaction) {
    return std::forward<F>(transaction)();
  }

  inline void NullWebSessionDataStore::close() {}
}

