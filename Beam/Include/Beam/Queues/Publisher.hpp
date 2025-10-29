#ifndef BEAM_PUBLISHER_HPP
#define BEAM_PUBLISHER_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"

namespace Beam {

  /** Base class for the parametric Publisher. */
  class BasePublisher {
    public:
      virtual ~BasePublisher() = default;

      /**
       * Synchronizes access to this publisher.
       * @param f The synchronized action to perform.
       */
      template<std::invocable<> F>
      decltype(auto) with(F&&) const;

      /**
       * Synchronizes access to this publisher.
       * @param f The synchronized action to perform.
       */
      virtual void with(const std::function<void ()>& f) const = 0;

    protected:

      /** Constructs a BasePublisher. */
      BasePublisher() = default;
  };

  /**
   * Interface for an object that publishes data to Queues.
   * @tparam T The type of data being published.
   */
  template<typename T>
  class Publisher : public BasePublisher {
    public:

      /** The type of data published. */
      using Type = T;

      /**
       * Monitors updates to this Publisher.
       * @param monitor The monitor to publish updates to.
       */
      virtual void monitor(ScopedQueueWriter<Type> monitor) const = 0;

    protected:

      /** Constructs a Publisher. */
      Publisher() = default;
  };

  /**
   * Specifies the type of values a Publisher produces.
   * @tparam P The type of Publisher to inspect.
   */
  template<typename P>
  struct publisher_type {
    using type = typename dereference_t<P>::Type;
  };

  template<typename P>
  using publisher_type_t = typename publisher_type<P>::type;

  template<std::invocable<> F>
  decltype(auto) BasePublisher::with(F&& f) const {
    using R = std::invoke_result_t<F>;
    using BaseWith =
      void (BasePublisher::*)(const std::function<void ()>&) const;
    if constexpr(std::is_reference_v<R>) {
      auto result = static_cast<std::remove_reference_t<R>*>(nullptr);
      (this->*static_cast<BaseWith>(&BasePublisher::with))([&] {
        result = &(std::forward<F>(f)());
      });
      return *result;
    } else if constexpr(std::is_void_v<R>) {
      (this->*static_cast<BaseWith>(&BasePublisher::with))(std::forward<F>(f));
    } else {
      auto result = boost::optional<R>();
      (this->*static_cast<BaseWith>(&BasePublisher::with))([&] {
        result.emplace(std::forward<F>(f)());
      });
      return R(std::move(*result));
    }
  }
}

#endif
