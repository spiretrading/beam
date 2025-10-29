#ifndef BEAM_UID_DATA_STORE_HPP
#define BEAM_UID_DATA_STORE_HPP
#include <concepts>
#include <cstdint>
#include <memory>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept for types that can be used as a UID data store. */
  template<typename T>
  concept IsUidDataStore = IsConnection<T> && requires(T& store) {
    { store.get_next_uid() } -> std::same_as<std::uint64_t>;
    { store.reserve(decltype(store.get_next_uid())()) } ->
        std::same_as<std::uint64_t>;
    { store.with_transaction(std::declval<const std::function<void ()>&>()) } ->
        std::same_as<void>;
  };

  /** Base class used to manage UIDs. */
  class UidDataStore {
    public:

      /**
       * Constructs a UidDataStore of a specified type using emplacement.
       * @tparam T The type of data store to emplace.
       * @param args The arguments to pass to the emplaced data store.
       */
      template<IsUidDataStore T, typename... Args>
      explicit UidDataStore(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a UidDataStore by referencing an existing data store.
       * @param data_store The data store to reference.
       */
      template<DisableCopy<UidDataStore> T> requires
        IsUidDataStore<dereference_t<T>>
      UidDataStore(T&& data_store);

      UidDataStore(const UidDataStore&) = default;

      /** Returns the next available UID. */
      std::uint64_t get_next_uid();

      /**
       * Reserves a block of UIDs.
       * @param size The size of the UID block to reserve.
       * @return The first UID in the reserved block.
       */
      std::uint64_t reserve(std::uint64_t size);

      /**
       * Performs an atomic transaction.
       * @param transaction The transaction to perform.
       */
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);

      /** Closes the data store. */
      void close();

    private:
      struct VirtualDataStore {
        virtual ~VirtualDataStore() = default;

        virtual std::uint64_t get_next_uid() = 0;
        virtual std::uint64_t reserve(std::uint64_t) = 0;
        virtual void with_transaction(const std::function<void ()>&) = 0;
        virtual void close() = 0;
      };
      template<typename D>
      struct WrappedDataStore final : VirtualDataStore {
        using DataStore = D;
        local_ptr_t<DataStore> m_data_store;

        template<typename... Args>
        WrappedDataStore(Args&&... args);

        std::uint64_t get_next_uid() override;
        std::uint64_t reserve(std::uint64_t size) override;
        void with_transaction(
          const std::function<void ()>& transaction) override;
        void close() override;
      };
      VirtualPtr<VirtualDataStore> m_data_store;
  };

  template<IsUidDataStore T, typename... Args>
  UidDataStore::UidDataStore(std::in_place_type_t<T>, Args&&... args)
    : m_data_store(make_virtual_ptr<WrappedDataStore<T>>(
        std::forward<Args>(args)...)) {}

  template<DisableCopy<UidDataStore> T> requires
    IsUidDataStore<dereference_t<T>>
  UidDataStore::UidDataStore(T&& data_store)
    : m_data_store(make_virtual_ptr<WrappedDataStore<std::remove_cvref_t<T>>>(
        std::forward<T>(data_store))) {}

  inline std::uint64_t UidDataStore::get_next_uid() {
    return m_data_store->get_next_uid();
  }

  inline std::uint64_t UidDataStore::reserve(std::uint64_t size) {
    return m_data_store->reserve(size);
  }

  template<std::invocable<> F>
  decltype(auto) UidDataStore::with_transaction(F&& transaction) {
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

  inline void UidDataStore::close() {
    m_data_store->close();
  }

  template<typename D>
  template<typename... Args>
  UidDataStore::WrappedDataStore<D>::WrappedDataStore(Args&&... args)
    : m_data_store(std::forward<Args>(args)...) {}

  template<typename D>
  std::uint64_t UidDataStore::WrappedDataStore<D>::get_next_uid() {
    return m_data_store->get_next_uid();
  }

  template<typename D>
  std::uint64_t UidDataStore::WrappedDataStore<D>::reserve(std::uint64_t size) {
    return m_data_store->reserve(size);
  }

  template<typename D>
  void UidDataStore::WrappedDataStore<D>::with_transaction(
      const std::function<void ()>& transaction) {
    m_data_store->with_transaction(transaction);
  }

  template<typename D>
  void UidDataStore::WrappedDataStore<D>::close() {
    m_data_store->close();
  }
}

#endif
