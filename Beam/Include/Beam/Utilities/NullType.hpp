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
    typedef ::Beam::NullType value_type;
    typedef ::Beam::NullType reference;
    typedef ::Beam::NullType const_reference;
    typedef ::Beam::NullType param_type;
  };
}

#endif
