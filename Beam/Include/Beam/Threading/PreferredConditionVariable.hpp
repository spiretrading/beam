#ifndef BEAM_PREFERREDCONDITIONVARIABLE_HPP
#define BEAM_PREFERREDCONDITIONVARIABLE_HPP
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \struct PreferredConditionVariable
      \brief Type trait used to specify the preferred type of condition variable
             to use with a mutex.
      \tparam MutexType The type of mutex to select the condition variable on.
   */
  template<typename MutexType>
  struct PreferredConditionVariable {
    using type = boost::condition_variable_any;
  };

  template<typename MutexType>
  using GetPreferredConditionVariable =
    typename PreferredConditionVariable<MutexType>::type;

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
}

#endif
