#ifndef BEAM_COLLECTIONS_HPP
#define BEAM_COLLECTIONS_HPP
#include <cstddef>

namespace Beam {
  template<typename T> class AnyIterator;
  template<typename T, std::size_t N> class Enum;
  template<typename T> class EnumIterator;
  template<typename T> class EnumSet;
  template<typename IteratorType> class IndexedIterator;
  template<typename IteratorType> class IndexedIteratorValue;
  template<typename T> class View;
}

#endif
