#ifndef BEAM_ENDIAN_HPP
#define BEAM_ENDIAN_HPP
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Returns <code>true</code> iff this platform uses a big-endian byte order.
  inline bool IsBigEndian() {
    static const int i = 1;
    return *reinterpret_cast<const char*>(&i) != 1;
  }

  //! Reverses the byte order of a value.
  /*!
    \param value The value whose bytes are to be reversed.
    \return The given <i>value</i> with its bytes reversed.
  */
  template<typename T>
  T ReverseBytes(T value) {
    T reversed;
    const char* i = reinterpret_cast<const char*>(&value) + sizeof(T) - 1;
    for(int j = 0; j < sizeof(T); ++j) {
      *(reinterpret_cast<char*>(&reversed) + j) = *i;
      --i;
    }
    return reversed;
  }

  //! Returns the native representation of a big endian value.
  /*!
    \param value The value.
    \return The native representation of the specified <i>value</i>.
  */
  template<typename T>
  T FromBigEndian(T value) {
    if(IsBigEndian()) {
      return value;
    }
    return ReverseBytes(value);
  }

  //! Returns the native representation of a little endian value.
  /*!
    \param value The value.
    \return The little-endian representati of the specified <i>value</i>.
  */
  template<typename T>
  T FromLittleEndian(T value) {
    if(!IsBigEndian()) {
      return value;
    }
    return ReverseBytes(value);
  }

  //! Returns the big-endian representation of a value.
  /*!
    \param value The value.
    \return The big-endian representation of the specified <i>value</i>.
  */
  template<typename T>
  T ToBigEndian(T value) {
    if(IsBigEndian()) {
      return value;
    }
    return ReverseBytes(value);
  }

  //! Returns the little-endian representation of a value.
  /*!
    \param value The value.
    \return The little-endian representation of the specified <i>value</i>.
  */
  template<typename T>
  T ToLittleEndian(T value) {
    if(!IsBigEndian()) {
      return value;
    }
    return ReverseBytes(value);
  }
}

#endif
