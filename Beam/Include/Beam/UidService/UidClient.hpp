#ifndef BEAM_UID_CLIENT_HPP
#define BEAM_UID_CLIENT_HPP
#include <cstdint>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"

namespace Beam {

  /** Concept for types that can be used as a UidClient. */
  template<typename T>
  concept IsUidClient = IsConnection<T> && requires(T& client) {
    { client.load_next_uid() } -> std::same_as<std::uint64_t>;
  };

  /** Client used to generate unique identifiers. */
  class UidClient {
    public:

      /**
       * Constructs a UidClient of a specified type using emplacement.
       * @tparam T The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<IsUidClient T, typename... Args>
      explicit UidClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a UidClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<DisableCopy<UidClient> T> requires IsUidClient<dereference_t<T>>
      UidClient(T&& client);

      UidClient(const UidClient&) = default;

      /** Returns the next available UID. */
      std::uint64_t load_next_uid();

      void close();

    private:
      struct VirtualUidClient {
        virtual ~VirtualUidClient() = default;

        virtual std::uint64_t load_next_uid() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedUidClient final : VirtualUidClient {
        using UidClient = C;
        local_ptr_t<UidClient> m_client;

        template<typename... Args>
        WrappedUidClient(Args&&... args);

        std::uint64_t load_next_uid() override;
        void close() override;
      };
      VirtualPtr<VirtualUidClient> m_client;
  };

  template<IsUidClient T, typename... Args>
  UidClient::UidClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(
        make_virtual_ptr<WrappedUidClient<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<UidClient> T> requires IsUidClient<dereference_t<T>>
  UidClient::UidClient(T&& client)
    : m_client(make_virtual_ptr<WrappedUidClient<std::remove_cvref_t<T>>>(
        std::forward<T>(client))) {}

  inline std::uint64_t UidClient::load_next_uid() {
    return m_client->load_next_uid();
  }

  inline void UidClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  UidClient::WrappedUidClient<C>::WrappedUidClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::uint64_t UidClient::WrappedUidClient<C>::load_next_uid() {
    return m_client->load_next_uid();
  }

  template<typename C>
  void UidClient::WrappedUidClient<C>::close() {
    m_client->close();
  }
}

#endif
