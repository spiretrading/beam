#ifndef BEAM_TIME_CLIENT_HPP
#define BEAM_TIME_CLIENT_HPP
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"

namespace Beam {

  /** Standard name for the time service. */
  inline const auto TIME_SERVICE_NAME = std::string("time_service");

  /** Determines whether a type meets the TimeClient concept. */
  template<typename T>
  concept IsTimeClient = IsConnection<T> && requires(T& client) {
    { client.get_time() } -> std::same_as<boost::posix_time::ptime>;
  };

  /** Retrieves the current time in UTC. */
  class TimeClient {
    public:

      /**
       * Constructs a TimeClient of a specified type using emplacement.
       * @tparam T The type of client to emplace.
       * @param args The arguments to pass to the emplaced client.
       */
      template<IsTimeClient T, typename... Args>
      explicit TimeClient(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a TimeClient by referencing an existing client.
       * @param client The client to reference.
       */
      template<DisableCopy<TimeClient> T> requires
        IsTimeClient<dereference_t<T>>
      TimeClient(T&& client);

      TimeClient(const TimeClient&) = default;

      /** Returns the current time in UTC. */
      boost::posix_time::ptime get_time();

      void close();
 
    private:
      struct VirtualTimeClient {
        virtual ~VirtualTimeClient() = default;

        virtual boost::posix_time::ptime get_time() = 0;
        virtual void close() = 0;
      };
      template<typename C>
      struct WrappedTimeClient final : VirtualTimeClient {
        using TimeClient = C;
        local_ptr_t<TimeClient> m_client;

        template<typename... Args>
        WrappedTimeClient(Args&&... args);

        boost::posix_time::ptime get_time() override;
        void close() override;
      };
      VirtualPtr<VirtualTimeClient> m_client;
 };

  /**
   * Truncates a time point.
   * @param time The time point to truncate.
   * @param unit The unit of time to truncate the time point to.
   * @return The time point truncated to the specified <i>unit</i>.
   */
  inline boost::posix_time::ptime truncate(
      boost::posix_time::ptime time, boost::posix_time::time_duration unit) {
    return time - boost::posix_time::microseconds(
      time.time_of_day().total_microseconds() % unit.total_microseconds());
  }

  template<IsTimeClient T, typename... Args>
  TimeClient::TimeClient(std::in_place_type_t<T>, Args&&... args)
    : m_client(
        make_virtual_ptr<WrappedTimeClient<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<TimeClient> T> requires IsTimeClient<dereference_t<T>>
  TimeClient::TimeClient(T&& client)
    : m_client(make_virtual_ptr<WrappedTimeClient<std::remove_cvref_t<T>>>(
        std::forward<T>(client))) {}

  inline boost::posix_time::ptime TimeClient::get_time() {
    return m_client->get_time();
  }

  inline void TimeClient::close() {
    m_client->close();
  }

  template<typename C>
  template<typename... Args>
  TimeClient::WrappedTimeClient<C>::WrappedTimeClient(Args&&... args)
    : m_client(std::forward<Args>(args)...) {}

  template<typename C>
  boost::posix_time::ptime TimeClient::WrappedTimeClient<C>::get_time() {
    return m_client->get_time();
  }

  template<typename C>
  void TimeClient::WrappedTimeClient<C>::close() {
    m_client->close();
  }
}

#endif
