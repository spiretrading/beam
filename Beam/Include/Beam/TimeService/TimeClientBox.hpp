#ifndef BEAM_TIME_CLIENT_BOX_HPP
#define BEAM_TIME_CLIENT_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/TimeService/TimeService.hpp"
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace TimeService {

  /** Provides a generic interface over an arbitrary TimeClient. */
  class TimeClientBox {
    public:

      /**
       * Constructs a TimeClientBox of a specified type using emplacement.
       * @param <T> The type of time client to emplace.
       * @param args The arguments to pass to the emplaced time client.
       */
      template<typename T, typename... Args>
      explicit TimeClientBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a TimeClientBox by copying an existing time client.
       * @param client The client to copy.
       */
      template<typename TimeClient>
      explicit TimeClientBox(TimeClient client);

      explicit TimeClientBox(TimeClientBox* client);

      explicit TimeClientBox(const std::shared_ptr<TimeClientBox>& client);

      explicit TimeClientBox(const std::unique_ptr<TimeClientBox>& client);

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      struct VirtualTimeClient {
        virtual ~VirtualTimeClient() = default;
        virtual boost::posix_time::ptime GetTime() = 0;
        virtual void Close() = 0;
      };
      template<typename C>
      struct WrappedTimeClient final : VirtualTimeClient {
        using TimeClient = C;
        GetOptionalLocalPtr<TimeClient> m_client;

        template<typename... Args>
        WrappedTimeClient(Args&&... args);
        boost::posix_time::ptime GetTime() override;
        void Close() override;
      };
      std::shared_ptr<VirtualTimeClient> m_client;
  };

  template<typename T, typename... Args>
  TimeClientBox::TimeClientBox(std::in_place_type_t<T>, Args&&... args)
    : m_client(std::make_shared<WrappedTimeClient<T>>(
        std::forward<Args>(args)...)) {}

  template<typename TimeClient>
  TimeClientBox::TimeClientBox(TimeClient client)
    : TimeClientBox(std::in_place_type<TimeClient>, std::move(client)) {}

  inline TimeClientBox::TimeClientBox(TimeClientBox* client)
    : TimeClientBox(*client) {}

  inline TimeClientBox::TimeClientBox(
    const std::shared_ptr<TimeClientBox>& client)
    : TimeClientBox(*client) {}

  inline TimeClientBox::TimeClientBox(
    const std::unique_ptr<TimeClientBox>& client)
    : TimeClientBox(*client) {}

  inline boost::posix_time::ptime TimeClientBox::GetTime() {
    return m_client->GetTime();
  }

  inline void TimeClientBox::Close() {
    m_client->Close();
  }

  template<typename C>
  template<typename... Args>
  TimeClientBox::WrappedTimeClient<C>::WrappedTimeClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  boost::posix_time::ptime TimeClientBox::WrappedTimeClient<C>::GetTime() {
    return m_client->GetTime();
  }

  template<typename C>
  void TimeClientBox::WrappedTimeClient<C>::Close() {
    m_client->Close();
  }
}

  template<>
  struct ImplementsConcept<TimeService::TimeClientBox,
    TimeService::TimeClient> : std::true_type {};
}

#endif
