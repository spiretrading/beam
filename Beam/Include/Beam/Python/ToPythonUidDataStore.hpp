#ifndef BEAM_TO_PYTHON_UID_DATA_STORE_HPP
#define BEAM_TO_PYTHON_UID_DATA_STORE_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam::Python {

  /**
   * Wraps a UidDataStore class for use within Python.
   * @tparam D The type of UidDataStore to wrap.
   */
  template<IsUidDataStore D>
  class ToPythonUidDataStore {
    public:

      /** The type of UidDataStore to wrap. */
      using DataStore = D;

      /**
       * Constructs a ToPythonUidDataStore in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonUidDataStore(Args&&... args);

      ~ToPythonUidDataStore();

      /** Returns a reference to the underlying data store. */
      DataStore& get();

      /** Returns a reference to the underlying data store. */
      const DataStore& get() const;

      std::uint64_t get_next_uid();
      std::uint64_t reserve(std::uint64_t size);
      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction);
      void close();

    private:
      boost::optional<DataStore> m_data_store;

      ToPythonUidDataStore(const ToPythonUidDataStore&) = delete;
      ToPythonUidDataStore& operator =(const ToPythonUidDataStore&) = delete;
  };

  template<typename DataStore>
  ToPythonUidDataStore(DataStore&&) ->
    ToPythonUidDataStore<std::remove_cvref_t<DataStore>>;

  template<IsUidDataStore D>
  template<typename... Args>
  ToPythonUidDataStore<D>::ToPythonUidDataStore(Args&&... args)
    : m_data_store((pybind11::gil_scoped_release(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsUidDataStore D>
  ToPythonUidDataStore<D>::~ToPythonUidDataStore() {
    auto release = pybind11::gil_scoped_release();
    m_data_store.reset();
  }

  template<IsUidDataStore D>
  typename ToPythonUidDataStore<D>::DataStore&
      ToPythonUidDataStore<D>::get() {
    return *m_data_store;
  }

  template<IsUidDataStore D>
  const typename ToPythonUidDataStore<D>::DataStore&
      ToPythonUidDataStore<D>::get() const {
    return *m_data_store;
  }

  template<IsUidDataStore D>
  std::uint64_t ToPythonUidDataStore<D>::get_next_uid() {
    auto release = pybind11::gil_scoped_release();
    return m_data_store->get_next_uid();
  }

  template<IsUidDataStore D>
  std::uint64_t ToPythonUidDataStore<D>::reserve(std::uint64_t size) {
    auto release = pybind11::gil_scoped_release();
    return m_data_store->reserve(size);
  }

  template<IsUidDataStore D>
  template<std::invocable<> F>
  decltype(auto) ToPythonUidDataStore<D>::with_transaction(F&& transaction) {
    auto release = pybind11::gil_scoped_release();
    return m_data_store->with_transaction(std::forward<F>(transaction));
  }

  template<IsUidDataStore D>
  void ToPythonUidDataStore<D>::close() {
    auto release = pybind11::gil_scoped_release();
    m_data_store->close();
  }
}

#endif
