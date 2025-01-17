#ifndef BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#define BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * Type trait used to specify the preferred type of condition variable to use
   * with a mutex.
   * @param <M> The type of mutex to select the condition variable on.
   */
  template<typename M>
  struct PreferredConditionVariable {
    using type = std::condition_variable_any;
  };

  template<typename M>
  using GetPreferredConditionVariable =
    typename PreferredConditionVariable<M>::type;

  template<>
  struct PreferredConditionVariable<std::mutex> {
    using type = std::condition_variable;
  };
}

#endif
