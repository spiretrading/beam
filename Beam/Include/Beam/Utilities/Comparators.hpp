#ifndef BEAM_COMPARATORS_HPP
#define BEAM_COMPARATORS_HPP
#include <functional>
#include <utility>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Returns a PropertyComparatorType.
  /*!
    \param property The method used to access the object's property.
  */
  template<typename Property>
  auto PropertyComparator(const Property& property) {
    return
      [property = std::mem_fn(property)](auto&& lhs, auto&& rhs) {
        return property(lhs) < property(rhs);
      };
  }
}

#endif
