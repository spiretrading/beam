#ifndef BEAM_TO_PYTHON_EMAIL_CLIENT_HPP
#define BEAM_TO_PYTHON_EMAIL_CLIENT_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/WebServices/EmailClient.hpp"

namespace Beam::Python {

  /**
   * Wraps an EmailClient for use with Python.
   * @tparam C The type of EmailClient to wrap.
   */
  template<IsEmailClient C>
  class ToPythonEmailClient {
    public:

      /** The type of EmailClient to wrap. */
      using EmailClient = C;

      /**
       * Constructs a ToPythonEmailClient in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonEmailClient(Args&&... args);

      ~ToPythonEmailClient();

      /** Returns a reference to the underlying email client. */
      EmailClient& get();

      /** Returns a reference to the underlying email client. */
      const EmailClient& get() const;

      /** Returns a reference to the underlying email client. */
      EmailClient& operator *();

      /** Returns a reference to the underlying email client. */
      const EmailClient& operator *() const;

      /** Returns a pointer to the underlying email client. */
      EmailClient* operator ->();

      /** Returns a pointer to the underlying email client. */
      const EmailClient* operator ->() const;

      void send(const Email& email);
      void close();

    private:
      boost::optional<EmailClient> m_client;

      ToPythonEmailClient(const ToPythonEmailClient&) = delete;
      ToPythonEmailClient& operator =(const ToPythonEmailClient&) = delete;
  };

  template<typename EmailClient>
  ToPythonEmailClient(EmailClient&&) ->
    ToPythonEmailClient<std::remove_cvref_t<EmailClient>>;

  template<IsEmailClient C>
  template<typename... Args>
  ToPythonEmailClient<C>::ToPythonEmailClient(Args&&... args)
    : m_client((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsEmailClient C>
  ToPythonEmailClient<C>::~ToPythonEmailClient() {
    auto release = GilRelease();
    m_client.reset();
  }

  template<IsEmailClient C>
  typename ToPythonEmailClient<C>::EmailClient&
      ToPythonEmailClient<C>::get() {
    return *m_client;
  }

  template<IsEmailClient C>
  const typename ToPythonEmailClient<C>::EmailClient&
      ToPythonEmailClient<C>::get() const {
    return *m_client;
  }

  template<IsEmailClient C>
  typename ToPythonEmailClient<C>::EmailClient&
      ToPythonEmailClient<C>::operator *() {
    return *m_client;
  }

  template<IsEmailClient C>
  const typename ToPythonEmailClient<C>::EmailClient&
      ToPythonEmailClient<C>::operator *() const {
    return *m_client;
  }

  template<IsEmailClient C>
  typename ToPythonEmailClient<C>::EmailClient*
      ToPythonEmailClient<C>::operator ->() {
    return m_client.get_ptr();
  }

  template<IsEmailClient C>
  const typename ToPythonEmailClient<C>::EmailClient*
      ToPythonEmailClient<C>::operator ->() const {
    return m_client.get_ptr();
  }

  template<IsEmailClient C>
  void ToPythonEmailClient<C>::send(const Email& email) {
    auto release = GilRelease();
    m_client->send(email);
  }

  template<IsEmailClient C>
  void ToPythonEmailClient<C>::close() {
    auto release = GilRelease();
    m_client->close();
  }
}

#endif
