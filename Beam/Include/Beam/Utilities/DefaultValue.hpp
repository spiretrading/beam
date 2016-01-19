#ifndef BEAM_DEFAULTVALUE_HPP
#define BEAM_DEFAULTVALUE_HPP
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \struct Default
      \brief Placeholder value that can be converted to any default constructed
             value.
   */
  struct Default {

    //! Implicitly converts this to a default constructed value.
    template<typename T>
    operator T () const;
  };

  template<typename T>
  Default::operator T () const {
    return T();
  }
}

#endif
