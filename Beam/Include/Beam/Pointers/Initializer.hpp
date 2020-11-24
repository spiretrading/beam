#ifndef BEAM_INITIALIZER_HPP
#define BEAM_INITIALIZER_HPP
#include <tuple>
#include <utility>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /**
   * Wraps parameters intended to initialize a value, typically stored by a
   * managed pointer.
   */
  template<typename... Args>
  struct Initializer {

    /** Stores the parameters that are used to initialize the value. */
    std::tuple<Args&&...> m_args;

    /**
     * Constructs an Initializer.
     * @param args The parameters used to initialize the value.
     */
    template<typename... ArgForwards>
    Initializer(ArgForwards&&... args);

    /** Returns the tuple storing the arguments. */
    std::tuple<Args&&...>& ToTuple();

    /** Returns the tuple storing the arguments. */
    const std::tuple<Args&&...>& ToTuple() const;
  };

  /**
   * Constructs an Initializer from a list of arguments.
   * @param args The list of arguments to use as an initializer.
   */
  template<typename... Args>
  auto Initialize(Args&&... args) {
    return Initializer<Args...>(std::forward<Args>(args)...);
  }

  template<typename... Args>
  template<typename... ArgForwards>
  Initializer<Args...>::Initializer(ArgForwards&&... args)
    : m_args(std::forward<ArgForwards>(args)...) {}

  template<typename... Args>
  std::tuple<Args&&...>& Initializer<Args...>::ToTuple() {
    return m_args;
  }

  template<typename... Args>
  const std::tuple<Args&&...>& Initializer<Args...>::ToTuple() const {
    return m_args;
  }
}

#endif
