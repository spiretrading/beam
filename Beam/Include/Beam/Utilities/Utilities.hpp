#ifndef BEAM_UTILITIES_HPP
#define BEAM_UTILITIES_HPP
#include <cstddef>

namespace Beam {
  template<typename T> class Active;
  class AssertionException;
  template<typename T> class CachedValue;
  template<typename T> class Capture;
  template<typename T> struct Concept;
  template<typename T> class Expect;
  template<std::size_t N> class FixedString;
  template<typename I, typename C> struct ImplementsConcept;
  template<typename K, typename V, typename M> class KeyValueCache;
  class NotImplementedException;
  class NotSupportedException;
  struct NullType;
  template<typename T, typename M> class Remote;
  template<typename T, typename B> class ResourcePool;
  template<typename T, typename B> class ScopedResource;
  template<typename T> class Singleton;
  template<typename T> struct StorageType;
  template<typename T> struct Stream;
  class Streamable;
}

#endif
