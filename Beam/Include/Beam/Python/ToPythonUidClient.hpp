#ifndef BEAM_TO_PYTHON_UID_CLIENT_HPP
#define BEAM_TO_PYTHON_UID_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/UidService/UidClient.hpp"

namespace Beam::Python {

  /**
   * Wraps a UidClient for use with Python.
   * @tparam C The type of UidClient to wrap.
   */
  template<IsUidClient C>
  class ToPythonUidClient {
    public:

      /** The type of UidClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonUidClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonUidClient(Args&&... args);

      ~ToPythonUidClient();

      /** Returns a reference to the underlying client. */
      Client& get();

      /** Returns a reference to the underlying client. */
      const Client& get() const;

      std::uint64_t load_next_uid();
      void close();

    private:
      boost::optional<Client> m_client;

      ToPythonUidClient(const ToPythonUidClient&) = delete;
      ToPythonUidClient& operator =(const ToPythonUidClient&) = delete;
  };

  template<typename UidClient>
  ToPythonUidClient(UidClient&&) ->
    ToPythonUidClient<std::remove_cvref_t<UidClient>>;

  template<IsUidClient C>
  template<typename... Args>
  ToPythonUidClient<C>::ToPythonUidClient(Args&&... args)
    : m_client((pybind11::gil_scoped_release(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsUidClient C>
  ToPythonUidClient<C>::~ToPythonUidClient() {
    auto release = pybind11::gil_scoped_release();
    m_client.reset();
  }

  template<IsUidClient C>
  typename ToPythonUidClient<C>::Client& ToPythonUidClient<C>::get() {
    return *m_client;
  }

  template<IsUidClient C>
  const typename ToPythonUidClient<C>::Client&
      ToPythonUidClient<C>::get() const {
    return *m_client;
  }

  template<IsUidClient C>
  std::uint64_t ToPythonUidClient<C>::load_next_uid() {
    auto release = pybind11::gil_scoped_release();
    return m_client->load_next_uid();
  }

  template<IsUidClient C>
  void ToPythonUidClient<C>::close() {
    auto release = pybind11::gil_scoped_release();
    m_client->close();
  }
}

#endif
