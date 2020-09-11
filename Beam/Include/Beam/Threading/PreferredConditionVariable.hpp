#ifndef BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#define BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * Type trait used to specify the preferred type of condition variable to use
   * with a mutex.
   * @param <M> The type of mutex to select the condition variable on.
   */
  template<typename M>
  struct PreferredConditionVariable {
    using type = boost::condition_variable_any;
  };

  template<typename M>
  using GetPreferredConditionVariable =
    typename PreferredConditionVariable<M>::type;

  template<>
  struct PreferredConditionVariable<boost::mutex> {
    using type = boost::condition_variable;
  };

  template<>
  struct PreferredConditionVariable<Mutex> {
    using type = ConditionVariable;
  };

  template<>
  struct PreferredConditionVariable<RecursiveMutex> {
    using type = ConditionVariable;
  };
}

#endif
