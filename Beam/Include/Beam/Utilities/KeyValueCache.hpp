#ifndef BEAM_KEY_VALUE_CACHE_HPP
#define BEAM_KEY_VALUE_CACHE_HPP
#include <functional>
#include <boost/thread/mutex.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"

namespace Beam {

  /**
   * Stores a cache of key value pairs.
   * @tparam K The type of key used to retrieve values.
   * @tparam V The type of value to store.
   * @tparam M The type of mutex used to synchronize access.
   */
  template<typename K, typename V, typename M = boost::mutex>
  class KeyValueCache {
    public:

      /** The type of key used to retrieve values. */
      using Key = K;

      /** The type of value to store. */
      using Value = V;

      /** The type of mutex used to synchronize access. */
      using Mutex = M;

      /**
       * The function signature used to load values not yet cached.
       * @param key The key used to retrieve the value.
       */
      using Source = std::function<Value (const Key& key)>;

      /** Constructs a KeyValueCache. */
      KeyValueCache() = default;

      /**
       * Constructs a KeyValueCache with a specified source.
       * @param source The function to call to load values not yet cached.
       */
      explicit KeyValueCache(Source source);

      /**
       * Loads a value from this cache.
       * @param key The key used to retrieve the value.
       * @return The value associated with the specified <i>key</i>.
       */
      const Value& load(const Key& key);

      /** Sets the function used to load values not yet cached. */
      void set_source(const Source& source);

    private:
      SynchronizedUnorderedMap<Key, Value, Mutex> m_cache;
      Source m_source;

      KeyValueCache(const KeyValueCache&) = delete;
      KeyValueCache& operator =(const KeyValueCache&) = delete;
  };

  template<typename K, typename V, typename M>
  KeyValueCache<K, V, M>::KeyValueCache(Source source)
    : m_source(std::move(source)) {}

  template<typename K, typename V, typename M>
  const typename KeyValueCache<K, V, M>::Value&
      KeyValueCache<K, V, M>::load(const Key& key) {
    return m_cache.get_or_insert(key, [&] {
      return m_source(key);
    });
  }

  template<typename K, typename V, typename M>
  void KeyValueCache<K, V, M>::set_source(const Source& source) {
    m_source = source;
  }
}

#endif
