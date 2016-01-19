#ifndef BEAM_ATOMICOPERATIONS_HPP
#define BEAM_ATOMICOPERATIONS_HPP
#include <atomic>
#include "Beam/Threading/Threading.hpp"

namespace Beam {

  //! Sets the value of an atomic object based on the result of a test.
  /*!
    \param object The atomic object to set.
    \param value The value to set the atomic object to.
    \param test A function consisting of two parameters, the atomic
           <i>object</i>'s most recent value and the <i>value</i> to set it to.
    \return <code>true</code> iff the atomic <i>object</i> was set.
  */
  template<typename T, typename F>
  bool TestAndSet(std::atomic<T>& object, T value, const F& test) {
    while(true) {
      auto store = object.load();
      if(test(object, value)) {
        return false;
      }
      if(std::atomic_compare_exchange_strong(&object, &store, value)) {
        return true;
      }
    }
  }
}

#endif
