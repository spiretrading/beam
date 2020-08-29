#ifndef BEAM_VIRTUALTIMECLIENT_HPP
#define BEAM_VIRTUALTIMECLIENT_HPP
#include <memory>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/TimeService/TimeService.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam {
namespace TimeService {

  /*! \class VirtualTimeClient
      \brief Provides a pure virtual interface to a TimeClient.
   */
  class VirtualTimeClient : private boost::noncopyable {
    public:
      virtual ~VirtualTimeClient() = default;

      virtual boost::posix_time::ptime GetTime() = 0;

      virtual void Close() = 0;

    protected:

      //! Constructs a VirtualTimeClient.
      VirtualTimeClient() = default;
  };

  /*! \class WrapperTimeClient
      \brief Wraps a TimeClient providing it with a virtual interface.
      \tparam ClientType The type of TimeClient to wrap.
   */
  template<typename ClientType>
  class WrapperTimeClient : public VirtualTimeClient {
    public:

      //! The TimeClient to wrap.
      using Client = GetTryDereferenceType<ClientType>;

      //! Constructs a WrapperTimeClient.
      /*!
        \param client The TimeClient to wrap.
      */
      template<typename TimeClientForward>
      WrapperTimeClient(TimeClientForward&& client);

      //! Returns the wrapped client.
      const Client& GetClient() const;

      //! Returns the wrapped client.
      Client& GetClient();

      virtual boost::posix_time::ptime GetTime() override;

      virtual void Close() override;

    private:
      GetOptionalLocalPtr<ClientType> m_client;
  };

  //! Wraps a TimeClient into a VirtualTimeClient.
  /*!
    \param client The client to wrap.
  */
  template<typename TimeClient>
  std::unique_ptr<VirtualTimeClient> MakeVirtualTimeClient(
      TimeClient&& client) {
    return std::make_unique<WrapperTimeClient<TimeClient>>(
      std::forward<TimeClient>(client));
  }

  //! Wraps a TimeClient into a VirtualTimeClient.
  /*!
    \param initializer Initializes client to wrap.
  */
  template<typename TimeClient, typename... Args>
  std::unique_ptr<VirtualTimeClient> MakeVirtualTimeClient(
      Initializer<Args...>&& initializer) {
    return std::make_unique<WrapperTimeClient<TimeClient>>(
      std::move(initializer));
  }

  template<typename ClientType>
  template<typename TimeClientForward>
  WrapperTimeClient<ClientType>::WrapperTimeClient(TimeClientForward&& client)
      : m_client{std::forward<TimeClientForward>(client)} {}

  template<typename ClientType>
  const typename WrapperTimeClient<ClientType>::Client&
      WrapperTimeClient<ClientType>::GetClient() const {
    return *m_client;
  }

  template<typename ClientType>
  typename WrapperTimeClient<ClientType>::Client&
      WrapperTimeClient<ClientType>::GetClient() {
    return *m_client;
  }

  template<typename ClientType>
  boost::posix_time::ptime WrapperTimeClient<ClientType>::GetTime() {
    return m_client->GetTime();
  }

  template<typename ClientType>
  void WrapperTimeClient<ClientType>::Close() {
    m_client->Close();
  }
}
}

#endif
