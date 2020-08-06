#ifndef BEAM_KEYVALUECACHE_HPP
#define BEAM_KEYVALUECACHE_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class KeyValueCache
      \brief Stores a cache of key value pairs.
      \tparam KeyType The type of key used to retrieve values.
      \tparam ValueType The type of value to store.
      \tparam MutexType The type of mutex used to synchronize access.
   */
  template<typename KeyType, typename ValueType,
    typename MutexType = boost::mutex>
  class KeyValueCache : private boost::noncopyable {
    public:

      //! The type of key used to retrieve values.
      using Key = KeyType;

      //! The type of value to store.
      using Value = ValueType;

      //! The type of mutex used to synchronize access.
      using Mutex = MutexType;

      //! The function signature used to load values not yet cached.
      using SourceFunction = std::function<Value (const Key& key)>;

      //! Constructs a KeyValueCache.
      KeyValueCache();

      //! Constructs a KeyValueCache with a specified source.
      /*!
        \param source The function to call to load values not yet cached.
      */
      KeyValueCache(SourceFunction source);

      //! Loads a value from this cache.
      /*!
        \param key The key used to retrieve the value.
        \return The value associated with the specified <i>key</i>.
      */
      const Value& Load(const Key& key);

      //! Sets the function used to load values not yet cached.
      void SetSource(SourceFunction source);

    private:
      SynchronizedUnorderedMap<Key, Value, Mutex> m_cache;
      SourceFunction m_source;
  };

  template<typename KeyType, typename ValueType, typename MutexType>
  KeyValueCache<KeyType, ValueType, MutexType>::KeyValueCache() {}

  template<typename KeyType, typename ValueType, typename MutexType>
  KeyValueCache<KeyType, ValueType, MutexType>::KeyValueCache(
      SourceFunction source)
      : m_source(std::move(source)) {}

  template<typename KeyType, typename ValueType, typename MutexType>
  const typename KeyValueCache<KeyType, ValueType, MutexType>::Value&
      KeyValueCache<KeyType, ValueType, MutexType>::Load(const Key& key) {
    return m_cache.GetOrInsert(key,
      [&] {
        return m_source(key);
      });
  }

  template<typename KeyType, typename ValueType, typename MutexType>
  void KeyValueCache<KeyType, ValueType, MutexType>::SetSource(
      SourceFunction source) {
    m_source = std::move(source);
  }
}

#endif
