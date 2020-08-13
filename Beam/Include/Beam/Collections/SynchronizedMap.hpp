#ifndef BEAM_SYNCHRONIZED_MAP_HPP
#define BEAM_SYNCHRONIZED_MAP_HPP
#include <tuple>
#include <unordered_map>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Collections/Collections.hpp"

namespace Beam {

  /**
   * Wraps a map container allowing for atomic operations to be performed on it.
   * @param <T> The type of map to wrap.
   * @param <M> The type of mutex used to synchronized this container.
   */
  template<typename T, typename M = boost::mutex>
  class SynchronizedMap : public boost::noncopyable {
    public:

      /** The type of map being wrapped. */
      using Map = T;

      /** The type of mutex used for synchronization. */
      using Mutex = M;

      /** The type of key stored by this map. */
      using Key = typename Map::key_type;

      /** The type of value stored by this map. */
      using Value = typename Map::mapped_type;

      /** Constructs an empty map. */
      SynchronizedMap() = default;

      /**
       * Finds a value with a specified key or default constructs it if it
       * isn't in the map.
       * @param key The key to search for.
       * @return The value associated with the specified <i>key</i>.
       */
      Value& Get(const Key& key);

      /**
       * Finds a value with a specified key or inserts it if it isn't in the
       * map.
       * @param key The key to search for.
       * @param valueBuilder A function to call that builds the value to insert
       *        into this map if there is no existing value associated with the
       *        specified <i>key</i>.
       * @return The value associated with the specified <i>key</i>.
       */
      template<typename F>
      Value& GetOrInsert(const Key& key, F&& valueBuilder);

      /**
       * Finds a value with a specified key or emplaces it if it isn't in the
       * map.
       * @param key The key to search for.
       * @param valueBuilder A function to call that emplaces the value into
       *        this map if there is no existing value associated with the
       *        specified <i>key</i>.
       * @return The value associated with the specified <i>key</i>.
       */
      template<typename F>
      Value& TestAndSet(const Key& key, F&& test);

      /**
       * Finds a value with a specified key.
       * @param key The key to search for.
       * @return The value associated with the <i>key</i>.
       */
      boost::optional<Value> FindValue(const Key& key) const;

      /**
       * Finds a value with a specified key.
       * @param key The key to search for.
       * @return The value associated with the <i>key</i>.
       */
      boost::optional<const Value&> Find(const Key& key) const;

      /**
       * Finds a value with a specified key.
       * @param key The key to search for.
       * @return The value associated with the <i>key</i>.
       */
      boost::optional<Value&> Find(const Key& key);

      /**
       * Inserts a key/value pair into the map.
       * @param key The key to insert.
       * @param value The value to associate with the <i>key</i>.
       * @return <code>true</code> iff the value was inserted, otherwise a value
       *         with the specified key already exists.
       */
      template<typename ValueForward>
      bool Insert(const Key& key, ValueForward&& value);

      /**
       * Updates an existing key/value pair.
       * @param key The key to update.
       * @param value The updated value to associate with the <i>key</i>.
       */
      template<typename ValueForward>
      void Update(const Key& key, ValueForward&& value);

      /** Clears the contents of this map. */
      void Clear();

      /**
       * Removes a value from this map.
       * @param key The key associated with the value to remove.
       */
      void Erase(const Key& key);

      /**
       * Swaps this map with another.
       * @param map The map to swap with.
       */
      void Swap(Map& map);

      /**
       * Performs a synchronized action with the map.
       * @param f The action to perform on the map.
       */
      template<typename F>
      decltype(auto) With(F&& f);

      /**
       * Performs a synchronized action with the map.
       * @param f The action to perform on the map.
       */
      template<typename F>
      decltype(auto) With(F&& f) const;

    private:
      mutable Mutex m_mutex;
      Map m_map;
  };

  /**
   * A SynchronizedMap using an std::unordered_map.
   * @param K The map's key.
   * @param V The map's value.
   * @param M The type of mutex used to synchronized this container.
   */
  template<typename K, typename V, typename M = boost::mutex>
  using SynchronizedUnorderedMap = SynchronizedMap<std::unordered_map<K, V>, M>;

  template<typename T, typename M>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::Get(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      valueIterator = m_map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key), std::forward_as_tuple()).first;
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  template<typename F>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::GetOrInsert(const Key& key, F&& valueBuilder) {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      valueIterator = m_map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(valueBuilder())).first;
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  template<typename F>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::TestAndSet(const Key& key, F&& test) {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      test(m_map);
      valueIterator = m_map.find(key);
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedMap<T, M>::Value>
      SynchronizedMap<T, M>::FindValue(const Key& key) const {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::none;
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  boost::optional<const typename SynchronizedMap<T, M>::Value&>
      SynchronizedMap<T, M>::Find(const Key& key) const {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::none;
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedMap<T, M>::Value&>
      SynchronizedMap<T, M>::Find(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::optional<Value&>();
    }
    return valueIterator->second;
  }

  template<typename T, typename M>
  template<typename ValueForward>
  bool SynchronizedMap<T, M>::Insert(const Key& key, ValueForward&& value) {
    auto lock = boost::lock_guard(m_mutex);
    return m_map.insert(
      std::pair(key, Value(std::forward<ValueForward>(value)))).second;
  }

  template<typename T, typename M>
  template<typename ValueForward>
  void SynchronizedMap<T, M>::Update(const Key& key, ValueForward&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_map[key] = std::forward<ValueForward>(value);
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::Clear() {
    auto map = Map();
    {
      auto lock = boost::lock_guard(m_mutex);
      map.swap(m_map);
    }
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::Erase(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    m_map.erase(key);
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::Swap(Map& map) {
    auto lock = boost::lock_guard(m_mutex);
    m_map.swap(map);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedMap<T, M>::With(F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    return f(m_map);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedMap<T, M>::With(F&& f) const {
    auto lock = boost::lock_guard(m_mutex);
    return f(m_map);
  }
}

#endif
