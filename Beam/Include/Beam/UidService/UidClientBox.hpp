#ifndef BEAM_UID_CLIENT_BOX_HPP
#define BEAM_UID_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam::UidService {

  /** Provides a generic interface over an arbitrary UidClient. */
  class UidClientBox {
    public:

      /**
       * Constructs a UidClientBox of a specified type using emplacement.
       * @param <T> The type of UID client to emplace.
       * @param args The arguments to pass to the emplaced UID client.
       */
      template<typename T, typename... Args>
      explicit UidClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a UidClientBox by copying an existing UID client.
       * @param client The client to copy.
       */
      template<typename UidClient>
      explicit UidClientBox(UidClient client);

      explicit UidClientBox(UidClientBox* client);

      explicit UidClientBox(const std::shared_ptr<UidClientBox>& client);

      explicit UidClientBox(const std::unique_ptr<UidClientBox>& client);

      std::uint64_t LoadNextUid();

      void Close();

    private:
      struct VirtualUidClient {
        virtual ~VirtualUidClient() = default;
        virtual std::uint64_t LoadNextUid() = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedUidClient final : VirtualUidClient {
        using UidClient = C;
        GetOptionalLocalPtr<UidClient> m_client;

        template<typename... Args>
        WrappedUidClient(Args&&... args);
        std::uint64_t LoadNextUid() override;
        void Close() override;
      };
      std::shared_ptr<VirtualUidClient> m_client;
  };

  template<typename T, typename... Args>
  UidClientBox::UidClientBox(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedUidClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename UidClient>
  UidClientBox::UidClientBox(UidClient client)
    : UidClientBox(std::in_place_type<UidClient>, std::move(client)) {}

  inline UidClientBox::UidClientBox(UidClientBox* client)
    : UidClientBox(*client) {}

  inline UidClientBox::UidClientBox(const std::shared_ptr<UidClientBox>& client)
    : UidClientBox(*client) {}

  inline UidClientBox::UidClientBox(const std::unique_ptr<UidClientBox>& client)
    : UidClientBox(*client) {}

  inline std::uint64_t UidClientBox::LoadNextUid() {
    return m_client->LoadNextUid();
  }

  inline void UidClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  UidClientBox::WrappedUidClient<C>::WrappedUidClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  std::uint64_t UidClientBox::WrappedUidClient<C>::LoadNextUid() {
    return m_client->LoadNextUid();
  }

  template<typename C>
  void UidClientBox::WrappedUidClient<C>::Close() {
    m_client->Close();
  }
}

#endif
