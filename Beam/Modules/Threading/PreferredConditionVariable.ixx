module;
#include "Prelude.hpp"

export module Beam:PreferredConditionVariable;

export namespace Beam {

  /**
   * Type trait used to specify the preferred type of condition variable to use
   * with a mutex.
   * @tparam M The type of mutex to select the condition variable on.
   */
  template<typename M>
  struct preferred_condition_variable {
    using type = boost::condition_variable_any;
  };

  template<typename M>
  using preferred_condition_variable_t =
    typename preferred_condition_variable<M>::type;

  template<>
  struct preferred_condition_variable<boost::mutex> {
    using type = boost::condition_variable;
  };

  template<>
  struct preferred_condition_variable<Mutex> {
    using type = ConditionVariable;
  };

  template<>
  struct preferred_condition_variable<RecursiveMutex> {
    using type = ConditionVariable;
  };
}

