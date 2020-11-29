#ifndef BEAM_TO_PYTHON_UID_CLIENT_HPP
#define BEAM_TO_PYTHON_UID_CLIENT_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/UidService/UidClientBox.hpp"

namespace Beam::UidService {

  /**
   * Wraps a UidClient for use with Python.
   * @param C The type of UidClient to wrap.
   */
  template<typename C>
  class ToPythonUidClient {
    public:

      /** The type of UidClient to wrap. */
      using Client = C;

      /**
       * Constructs a ToPythonUidClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonUidClient(Args&&... args);

      ToPythonUidClient(ToPythonUidClient&&) = default;

      ~ToPythonUidClient();

      /** Returns the wrapped UidClient. */
      const Client& GetClient() const;

      /** Returns the wrapped UidClient. */
      Client& GetClient();

      std::uint64_t LoadNextUid();

      void Close();

      ToPythonUidClient& operator =(ToPythonUidClient&&) = default;

    private:
      boost::optional<Client> m_client;

      ToPythonUidClient(const ToPythonUidClient&) = delete;
      ToPythonUidClient& operator =(const ToPythonUidClient&) = delete;
  };

  template<typename UidClient>
  ToPythonUidClient(UidClient&&) -> ToPythonUidClient<std::decay_t<UidClient>>;

  template<typename C>
  template<typename... Args>
  ToPythonUidClient<C>::ToPythonUidClient(Args&&... args)
    : m_client((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonUidClient<C>::~ToPythonUidClient() {
    auto release = Python::GilRelease();
    m_client.reset();
  }

  template<typename C>
  const typename ToPythonUidClient<C>::Client&
      ToPythonUidClient<C>::GetClient() const {
    return *m_client;
  }

  template<typename C>
  typename ToPythonUidClient<C>::Client& ToPythonUidClient<C>::GetClient() {
    return *m_client;
  }

  template<typename C>
  std::uint64_t ToPythonUidClient<C>::LoadNextUid() {
    auto release = Python::GilRelease();
    return m_client->LoadNextUid();
  }

  template<typename C>
  void ToPythonUidClient<C>::Close() {
    auto release = Python::GilRelease();
    m_client->Close();
  }
}

#endif
