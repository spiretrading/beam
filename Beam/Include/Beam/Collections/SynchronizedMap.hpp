#ifndef BEAM_SYNCHRONIZED_MAP_HPP
#define BEAM_SYNCHRONIZED_MAP_HPP
#include <concepts>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a map container allowing for atomic operations to be performed on it.
   * @tparam T The type of map to wrap.
   * @tparam M The type of mutex used to synchronized this container.
   */
  template<typename T, typename M = boost::mutex>
  class SynchronizedMap {
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
       * Copies a map.
       * @param map The map to copy.
       */
      template<typename U, typename V> requires std::constructible_from<
        typename T::mapped_type, const typename U::mapped_type&>
      SynchronizedMap(const SynchronizedMap<U, V>& map);

      SynchronizedMap(const SynchronizedMap& map);
      SynchronizedMap(SynchronizedMap&& map);

      /**
       * Finds a value with a specified key or default constructs it if it
       * isn't in the map.
       * @param key The key to search for.
       * @return The value associated with the specified <i>key</i>.
       */
      Value& get(const Key& key);

      /**
       * Finds a value with a specified key or inserts it if it isn't in the
       * map.
       * @param key The key to search for.
       * @param builder A function to call that builds the value to insert into
       *        this map if there is no existing value associated with the
       *        specified <i>key</i>.
       * @return The value associated with the specified <i>key</i>.
       */
      template<std::invocable F>
      Value& get_or_insert(const Key& key, F&& builder);

      /**
       * Finds a value with a specified key or emplaces it if it isn't in the
       * map.
       * @param key The key to search for.
       * @param builder A function to call that emplaces the value into this map
       *        if there is no existing value associated with the specified
       *        <i>key</i>.
       * @return The value associated with the specified <i>key</i>.
       */
      template<std::invocable<T&> F>
      Value& test_and_set(const Key& key, F&& test);

      /**
       * Finds a value with a specified key and returns a copy.
       * @param key The key to search for.
       * @return A copy of the value associated with the <i>key</i>.
       */
      boost::optional<Value> try_load(const Key& key) const;

      /**
       * Finds a value with a specified key.
       * @param key The key to search for.
       * @return The value associated with the <i>key</i>.
       */
      boost::optional<const Value&> find(const Key& key) const;

      /**
       * Finds a value with a specified key.
       * @param key The key to search for.
       * @return The value associated with the <i>key</i>.
       */
      boost::optional<Value&> find(const Key& key);

      /**
       * Inserts a key/value pair into the map.
       * @param key The key to insert.
       * @param value The value to associate with the <i>key</i>.
       * @return <code>true</code> iff the value was inserted, otherwise a value
       *         with the specified key already exists.
       */
      template<typename V> requires
        std::constructible_from<typename T::mapped_type, V>
      bool insert(const Key& key, V&& value);

      /**
       * Updates an existing key/value pair.
       * @param key The key to update.
       * @param value The updated value to associate with the <i>key</i>.
       */
      template<typename V> requires
        std::constructible_from<typename T::mapped_type, V>
      void update(const Key& key, V&& value);

      /** Performs an action on each element of this map. */
      template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
      void for_each(this Self&& self, F f);

      /** Performs an action on each value in this map. */
      template<typename Self, IsInvocableLike<Self, typename T::mapped_type> F>
      void for_each_value(this Self&& self, F f);

      /** Clears the contents of this map. */
      void clear();

      /**
       * Removes a value from this map.
       * @param key The key associated with the value to remove.
       */
      void erase(const Key& key);

      /**
       * Swaps this map with another.
       * @param map The map to swap with.
       */
      void swap(Map& map);

      /**
       * Performs a synchronized action with the map.
       * @param f The action to perform on the map.
       */
      template<typename Self, IsInvocableLike<Self, T> F>
      decltype(auto) with(this Self&& self, F&& f);

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
  template<typename U, typename V>
    requires std::constructible_from<typename T::mapped_type,
      const typename U::mapped_type&>
  SynchronizedMap<T, M>::SynchronizedMap(const SynchronizedMap<U, V>& map) {
    auto lock = boost::lock_guard(map.m_mutex);
    m_map.insert(m_map.end(), map.m_map.begin(), map.m_map.end());
  }

  template<typename T, typename M>
  SynchronizedMap<T, M>::SynchronizedMap(const SynchronizedMap& map) {
    auto lock = boost::lock_guard(map.m_mutex);
    m_map = map.m_map;
  }

  template<typename T, typename M>
  SynchronizedMap<T, M>::SynchronizedMap(SynchronizedMap&& map) {
    auto lock = boost::lock_guard(map.m_mutex);
    m_map = std::move(map.m_map);
  }

  template<typename T, typename M>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::get(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      i = m_map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
        std::forward_as_tuple()).first;
    }
    return i->second;
  }

  template<typename T, typename M>
  template<std::invocable F>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::get_or_insert(const Key& key, F&& builder) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      i = m_map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
        std::forward_as_tuple(std::forward<F>(builder)())).first;
    }
    return i->second;
  }

  template<typename T, typename M>
  template<std::invocable<T&> F>
  typename SynchronizedMap<T, M>::Value&
      SynchronizedMap<T, M>::test_and_set(const Key& key, F&& test) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      std::forward<F>(test)(m_map);
      i = m_map.find(key);
    }
    return i->second;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedMap<T, M>::Value>
      SynchronizedMap<T, M>::try_load(const Key& key) const {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      return boost::none;
    }
    return i->second;
  }

  template<typename T, typename M>
  boost::optional<const typename SynchronizedMap<T, M>::Value&>
      SynchronizedMap<T, M>::find(const Key& key) const {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      return boost::none;
    }
    return i->second;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedMap<T, M>::Value&>
      SynchronizedMap<T, M>::find(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_map.find(key);
    if(i == m_map.end()) {
      return boost::none;
    }
    return i->second;
  }

  template<typename T, typename M>
  template<typename V> requires
    std::constructible_from<typename T::mapped_type, V>
  bool SynchronizedMap<T, M>::insert(const Key& key, V&& value) {
    auto lock = boost::lock_guard(m_mutex);
    auto [i, inserted] = m_map.try_emplace(key, std::forward<V>(value));
    return inserted;
  }

  template<typename T, typename M>
  template<typename V> requires
    std::constructible_from<typename T::mapped_type, V>
  void SynchronizedMap<T, M>::update(const Key& key, V&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_map.insert_or_assign(key, std::forward<V>(value));
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::clear() {
    auto map = Map();
    {
      auto lock = boost::lock_guard(m_mutex);
      map.swap(m_map);
    }
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::erase(const Key& key) {
    auto lock = boost::lock_guard(m_mutex);
    m_map.erase(key);
  }

  template<typename T, typename M>
  void SynchronizedMap<T, M>::swap(Map& map) {
    auto lock = boost::lock_guard(m_mutex);
    m_map.swap(map);
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, T> F>
  decltype(auto) SynchronizedMap<T, M>::with(this Self&& self, F&& f) {
    auto lock = boost::lock_guard(self.m_mutex);
    return std::forward<F>(f)(std::forward<Self>(self).m_map);
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
  void SynchronizedMap<T, M>::for_each(this Self&& self, F f) {
    auto lock = boost::lock_guard(self.m_mutex);
    if constexpr(std::is_const_v<std::remove_reference_t<Self>> ||
        std::is_lvalue_reference_v<Self>) {
      for(auto& entry : self.m_map) {
        f(entry);
      }
    } else {
      for(auto&& entry : self.m_map) {
        f(std::move(entry));
      }
    }
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, typename T::mapped_type> F>
  void SynchronizedMap<T, M>::for_each_value(this Self&& self, F f) {
    auto lock = boost::lock_guard(self.m_mutex);
    if constexpr(std::is_const_v<std::remove_reference_t<Self>> ||
        std::is_lvalue_reference_v<Self>) {
      for(auto& entry : self.m_map) {
        f(entry.second);
      }
    } else {
      for(auto&& entry : self.m_map) {
        f(std::move(entry.second));
      }
    }
  }
}

#endif
