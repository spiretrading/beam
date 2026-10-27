#ifndef BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#define BEAM_PREFERRED_CONDITION_VARIABLE_HPP
#include <condition_variable>
#include <mutex>

namespace Beam {

  /**
   * Type trait used to specify the preferred type of condition variable to use
   * with a mutex.
   * @tparam M The type of mutex to select the condition variable on.
   */
  template<typename M>
  struct preferred_condition_variable {
    using type = std::condition_variable_any;
  };

  template<typename M>
  using preferred_condition_variable_t =
    typename preferred_condition_variable<M>::type;

  template<>
  struct preferred_condition_variable<std::mutex> {
    using type = std::condition_variable;
  };
}

#endif
