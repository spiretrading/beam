#ifndef BEAM_NULLTYPE_HPP
#define BEAM_NULLTYPE_HPP
#include <boost/call_traits.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class NullType
      \brief Dummy structured used as a default template parameter.
   */
  struct NullType {};
}

namespace boost {
  template<>
  struct call_traits< ::Beam::NullType> {
    using value_type = ::Beam::NullType;
    using reference = ::Beam::NullType;
    using const_reference = ::Beam::NullType;
    using param_type = ::Beam::NullType;
  };
}

#endif
