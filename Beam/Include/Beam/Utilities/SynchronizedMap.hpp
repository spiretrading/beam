#ifndef BEAM_SYNCHRONIZEDMAP_HPP
#define BEAM_SYNCHRONIZEDMAP_HPP
#include <tuple>
#include <unordered_map>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class SynchronizedMap
      \brief Wraps a map container allowing for atomic operations to be
             performed on it.
      \tparam MapType The type of map to wrap.
      \tparam MutexType The type of mutex used to synchronized this container.
   */
  template<typename MapType, typename MutexType = boost::mutex>
  class SynchronizedMap : public boost::noncopyable {
    public:

      //! The type of map being wrapped.
      typedef MapType Map;

      //! The type of mutex used for synchronization.
      typedef MutexType Mutex;

      //! The type of key stored by this map.
      typedef typename Map::key_type Key;

      //! The type of value stored by this map.
      typedef typename Map::mapped_type Value;

      //! Constructs an empty map.
      SynchronizedMap();

      //! Finds a value with a specified key or default constructs it if it
      //! isn't in the map.
      /*!
        \param key The key to search for.
        \return The value associated with the specified <i>key</i>.
      */
      Value& Get(const Key& key);

      //! Finds a value with a specified key or inserts it if it isn't in the
      //! map.
      /*!
        \param key The key to search for.
        \param valueBuilder A function to call that builds the value to insert
               into this map if there is no existing value associated with the
               specified <i>key</i>.
        \return The value associated with the specified <i>key</i>.
      */
      template<typename F>
      Value& GetOrInsert(const Key& key, F valueBuilder);

      //! Finds a value with a specified key or emplaces it if it isn't in the
      //! map.
      /*!
        \param key The key to search for.
        \param valueBuilder A function to call that emplaces the value into this
               map if there is no existing value associated with the specified
               <i>key</i>.
        \return The value associated with the specified <i>key</i>.
      */
      template<typename F>
      Value& TestAndSet(const Key& key, F test);

      //! Finds a value with a specified key.
      /*!
        \param key The key to search for.
        \return The value associated with the <i>key</i>.
      */
      boost::optional<Value> FindValue(const Key& key) const;

      //! Finds a value with a specified key.
      /*!
        \param key The key to search for.
        \return The value associated with the <i>key</i>.
      */
      boost::optional<const Value&> Find(const Key& key) const;

      //! Finds a value with a specified key.
      /*!
        \param key The key to search for.
        \return The value associated with the <i>key</i>.
      */
      boost::optional<Value&> Find(const Key& key);

      //! Inserts a key/value pair into the map.
      /*!
        \param key The key to insert.
        \param value The value to associate with the <i>key</i>.
        \return <code>true</code> iff the value was inserted, otherwise a value
                with the specified key already exists.
      */
      template<typename ValueForward>
      bool Insert(const Key& key, ValueForward&& value);

      //! Updates an existing key/value pair.
      /*!
        \param key The key to update.
        \param value The updated value to associate with the <i>key</i>.
      */
      template<typename ValueForward>
      void Update(const Key& key, ValueForward&& value);

      //! Clears the contents of this map.
      void Clear();

      //! Removes a value from this map.
      /*!
        \param key The key associated with the value to remove.
      */
      void Erase(const Key& key);

      //! Swaps this map with another.
      /*!
        \param map The map to swap with.
      */
      void Swap(Map& map);

      //! Performs a synchronized action with the map.
      /*!
        \param f The action to perform on the map.
      */
      template<typename F>
      void With(F f);

      //! Performs a synchronized action with the map.
      /*!
        \param f The action to perform on the map.
      */
      template<typename F>
      void With(F f) const;

    private:
      mutable Mutex m_mutex;
      Map m_map;
  };

  /*! \class SynchronizedUnorderedMap
      \brief A SynchronizedMap using an std::unordered_map.
      \tparam KeyType The map's key.
      \tparam ValueType The map's value.
      \tparam MutexType The type of mutex used to synchronized this container.
   */
  template<typename KeyType, typename ValueType,
    typename MutexType = boost::mutex>
  using SynchronizedUnorderedMap = SynchronizedMap<
    std::unordered_map<KeyType, ValueType>, MutexType>;

  template<typename MapType, typename MutexType>
  SynchronizedMap<MapType, MutexType>::SynchronizedMap() {}

  template<typename MapType, typename MutexType>
  typename SynchronizedMap<MapType, MutexType>::Value&
      SynchronizedMap<MapType, MutexType>::Get(const Key& key) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      valueIterator = m_map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key), std::forward_as_tuple()).first;
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  template<typename F>
  typename SynchronizedMap<MapType, MutexType>::Value&
      SynchronizedMap<MapType, MutexType>::GetOrInsert(const Key& key,
      F valueBuilder) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      valueIterator = m_map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(valueBuilder())).first;
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  template<typename F>
  typename SynchronizedMap<MapType, MutexType>::Value&
      SynchronizedMap<MapType, MutexType>::TestAndSet(const Key& key, F test) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      test(m_map);
      valueIterator = m_map.find(key);
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  boost::optional<typename SynchronizedMap<MapType, MutexType>::Value>
      SynchronizedMap<MapType, MutexType>::FindValue(const Key& key) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::optional<Value>();
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  boost::optional<const typename SynchronizedMap<MapType, MutexType>::Value&>
      SynchronizedMap<MapType, MutexType>::Find(const Key& key) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::optional<const Value&>();
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  boost::optional<typename SynchronizedMap<MapType, MutexType>::Value&>
      SynchronizedMap<MapType, MutexType>::Find(const Key& key) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto valueIterator = m_map.find(key);
    if(valueIterator == m_map.end()) {
      return boost::optional<Value&>();
    }
    return valueIterator->second;
  }

  template<typename MapType, typename MutexType>
  template<typename ValueForward>
  bool SynchronizedMap<MapType, MutexType>::Insert(const Key& key,
      ValueForward&& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    return m_map.insert(
      std::make_pair(key, Value(std::forward<ValueForward>(value)))).second;
  }

  template<typename MapType, typename MutexType>
  template<typename ValueForward>
  void SynchronizedMap<MapType, MutexType>::Update(const Key& key,
      ValueForward&& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_map[key] = std::forward<ValueForward>(value);
  }

  template<typename MapType, typename MutexType>
  void SynchronizedMap<MapType, MutexType>::Clear() {
    Map map;
    {
      boost::lock_guard<Mutex> lock(m_mutex);
      map.swap(m_map);
    }
  }

  template<typename MapType, typename MutexType>
  void SynchronizedMap<MapType, MutexType>::Erase(const Key& key) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_map.erase(key);
  }

  template<typename MapType, typename MutexType>
  void SynchronizedMap<MapType, MutexType>::Swap(Map& map) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_map.swap(map);
  }

  template<typename MapType, typename MutexType>
  template<typename F>
  void SynchronizedMap<MapType, MutexType>::With(F f) {
    boost::lock_guard<Mutex> lock(m_mutex);
    f(m_map);
  }

  template<typename MapType, typename MutexType>
  template<typename F>
  void SynchronizedMap<MapType, MutexType>::With(F f) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    f(m_map);
  }
}

#endif
