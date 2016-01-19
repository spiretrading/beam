#ifndef BEAM_UNIQUEPTR_HPP
#define BEAM_UNIQUEPTR_HPP
#include <memory>

namespace Beam {

  //! Statically casts a unique_ptr.
  /*!
    \param source The source to cast.
    \return The <i>source</i> statically cast to a unique_ptr of type T.
  */
  template<typename T, typename U>
  std::unique_ptr<T> UniqueStaticCast(std::unique_ptr<U>&& source) {
    return std::unique_ptr<T>(static_cast<T*>(source.release()));
  }
}

#endif
