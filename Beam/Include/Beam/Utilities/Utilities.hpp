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
  template<typename InstanceType, typename ConceptType>
    struct ImplementsConcept;
  template<typename KeyType, typename ValueType, typename MutexType>
    class KeyValueCache;
  class NotImplementedException;
  class NotSupportedException;
  struct NullType;
  template<typename T, typename MutexType> class Remote;
  template<typename T> class ResourcePool;
  template<typename T> class ScopedResource;
  template<typename T> class Singleton;
  template<typename T, typename ComparatorType, typename AllocatorType>
    class SortedVector;
  template<typename T> struct StorageType;
  template<typename T> struct Stream;
  class Streamable;
  template<typename ListType, typename MutexType> class SynchronizedList;
  template<typename MapType, typename MutexType> class SynchronizedMap;
  template<typename SetType, typename MutexType> class SynchronizedSet;
}

#endif
