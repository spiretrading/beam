#ifndef BEAM_RETHROW_HPP
#define BEAM_RETHROW_HPP
#include <exception>

namespace Beam {

  //! Rethrows an exception or else a no-op if the exception is equal to the
  //! nullptr.
  /*!
    \param e The exception to rethrow.
  */
  inline void Rethrow(const std::exception_ptr& e) {
    if(!(e == nullptr)) {
      std::rethrow_exception(e);
    }
  }
}

#endif
