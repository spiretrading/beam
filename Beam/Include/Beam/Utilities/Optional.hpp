#ifndef BEAM_OPTIONAL_HPP
#define BEAM_OPTIONAL_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Tests if an optional is initialized and equals a specified value.
  /*!
    \param lhs The optional value to test.
    \param rhs The value to test for equality.
    \return <code>true</code> iff <i>lhs</i> is initialized and its value is
            equal to <i>rhs</i>.
  */
  template<typename T, typename Q>
  bool IsEqual(const boost::optional<T>& lhs, const Q& rhs) {
    return lhs.is_initialized() && *lhs == rhs;
  }
}

#endif
