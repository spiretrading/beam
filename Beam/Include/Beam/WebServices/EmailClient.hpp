#ifndef BEAM_EMAIL_CLIENT_HPP
#define BEAM_EMAIL_CLIENT_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/WebServices/Email.hpp"

namespace Beam {

  /** Concept for types that can be used as an EmailClient. */
  template<typename T>
  concept IsEmailClient = IsConnection<T> && requires(T& client) {
    { client.send(std::declval<const Email&>()) } -> std::same_as<void>;
  };

  /** Client used to send email messages. */
  class EmailClient {
    public:

      /**
       * Constructs an EmailClient of a specified type using emplacement.
       * @tparam T The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<IsEmailClient T, typename... Args>
      explicit EmailClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an EmailClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<DisableCopy<EmailClient> T> requires
        IsEmailClient<dereference_t<T>>
      EmailClient(T&& client);

      EmailClient(const EmailClient&) = default;

      /**
       * Sends an email.
       * @param email The email to send.
       */
      void send(const Email& email);

      void close();

    private:
      struct VirtualEmailClient {
        virtual ~VirtualEmailClient() = default;

        virtual void send(const Email& email) = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedEmailClient final : VirtualEmailClient {
        using EmailClient = C;
        local_ptr_t<EmailClient> m_client;

        template<typename... Args>
        WrappedEmailClient(Args&&... args);

        void send(const Email& email) override;
        void close() override;
      };
      VirtualPtr<VirtualEmailClient> m_client;
  };

  template<IsEmailClient T, typename... Args>
  EmailClient::EmailClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(
        make_virtual_ptr<WrappedEmailClient<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<EmailClient> T> requires IsEmailClient<dereference_t<T>>
  EmailClient::EmailClient(T&& client)
    : m_client(make_virtual_ptr<WrappedEmailClient<std::remove_cvref_t<T>>>(
        std::forward<T>(client))) {}

  inline void EmailClient::send(const Email& email) {
    m_client->send(email);
  }

  inline void EmailClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  EmailClient::WrappedEmailClient<C>::WrappedEmailClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  void EmailClient::WrappedEmailClient<C>::send(const Email& email) {
    m_client->send(email);
  }

  template<typename C>
  void EmailClient::WrappedEmailClient<C>::close() {
    m_client->close();
  }
}

#endif
