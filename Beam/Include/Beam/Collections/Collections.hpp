#ifndef BEAM_COLLECTIONS_HPP
#define BEAM_COLLECTIONS_HPP
#include <cstddef>

namespace Beam {
  template<typename T> class AnyIterator;
  template<typename T, std::size_t N> class Enum;
  template<typename T> class EnumIterator;
  template<typename T> class EnumSet;
  template<typename I> class IndexIterator;
  template<typename I> class IndexIteratorValue;
  template<typename T, typename C, typename A> class SortedVector;
  template<typename T, typename M> class SynchronizedList;
  template<typename T, typename M> class SynchronizedMap;
  template<typename T, typename M> class SynchronizedSet;
  template<typename T> class View;
}

#endif
