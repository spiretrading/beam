#ifndef BEAM_SYNCHRONIZED_SET_HPP
#define BEAM_SYNCHRONIZED_SET_HPP
#include <mutex>
#include <unordered_set>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Collections/Collections.hpp"

namespace Beam {

  /**
   * Wraps a set container allowing for atomic operations to be performed on it.
   * @param <T> The type of set to wrap.
   * @param <M> The type of mutex used for synchronization.
   */
  template<typename T, typename M = std::mutex>
  class SynchronizedSet {
    public:

      /** The type of set being wrapped. */
      using Set = T;

      /** The type of mutex used for synchronization. */
      using Mutex = M;

      /** The type of value stored by this set. */
      using Value = typename Set::value_type;

      /** Constructs an empty set. */
      SynchronizedSet() = default;

      /**
       * Returns <code>true</code> iff this set contains a specified value.
       * @param value The value to find.
       * @return <code>true</code> iff this set contains the <i>value</i>.
       */
      bool Contains(const Value& value) const;

      /**
       * Finds a value.
       * @param value The value to search for.
       * @return The value contained in this set.
       */
      boost::optional<Value> FindValue(const Value& value) const;

      /**
       * Finds a value.
       * @param value The value to search for.
       * @return The value contained in this set.
       */
      boost::optional<const Value&> Find(const Value& value) const;

      /**
       * Finds a value.
       * @param value The value to search for.
       * @return The value contained in this set.
       */
      boost::optional<Value&> Find(const Value& value);

      /**
       * Returns a value stored by this set or inserts it if it isn't in the
       * set.
       * @param value The value to search for.
       * @return The value stored in the set.
       */
      Value Get(const Value& value);

      /**
       * Tests if a value is in this set, and if it isn't then inserts it and
       * calls the supplied function.
       * @param value The value to test for.
       * @param f The function to call if the value is not found.
       */
      template<typename F>
      void TestAndSet(const Value& value, F&& f);

      /**
       * Inserts a value into the set.
       * @param value The value to insert.
       */
      template<typename V>
      bool Insert(V&& value);

      /**
       * Emplaces a value into the set.
       * @param value The value to insert.
       */
      template<typename... Args>
      bool Emplace(Args&&... args);

      /**
       * Updates a value that may or may not be contained in the set.
       * @param value The updated value.
       */
      void Update(const Value& value);

      /** Clears the contents of this set. */
      void Clear();

      /**
       * Removes a value from this set.
       * @param value The value to remove.
       */
      void Erase(const Value& value);

      /**
       * Swaps this set with another.
       * @param set The set to swap with.
       */
      void Swap(Set& set);

      /**
       * Performs a synchronized action with the set.
       * @param f The action to perform on the set.
       */
      template<typename F>
      decltype(auto) With(F&& f);

      /**
       * Performs a synchronized action with the set.
       * @param f The action to perform on the set.
       */
      template<typename F>
      decltype(auto) With(F&& f) const;

    private:
      mutable Mutex m_mutex;
      Set m_set;

      SynchronizedSet(const SynchronizedSet&) = delete;
      SynchronizedSet& operator =(const SynchronizedSet&) = delete;
  };

  /**
   * A SynchronizedSet using an std::unordered_set.
   * @param K The set's key.
   * @param M The type of mutex used to synchronized this container.
   */
  template<typename K, typename M = std::mutex>
  using SynchronizedUnorderedSet = SynchronizedSet<std::unordered_set<K>, M>;

  template<typename T, typename M>
  bool SynchronizedSet<T, M>::Contains(const Value& value) const {
    auto lock = std::lock_guard(m_mutex);
    return m_set.find(value) != m_set.end();
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedSet<T, M>::Value>
      SynchronizedSet<T, M>::FindValue(const Value& value) const {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_set.find(value);
    if(i == m_set.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T, typename M>
  boost::optional<const typename SynchronizedSet<T, M>::Value&>
      SynchronizedSet<T, M>::Find(const Value& value) const {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_set.find(value);
    if(i == m_set.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedSet<T, M>::Value&>
      SynchronizedSet<T, M>::Find(const Value& value) {
    auto lock = std::lock_guard(m_mutex);
    auto i = m_set.find(value);
    if(i == m_set.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T, typename M>
  typename SynchronizedSet<T, M>::Value
      SynchronizedSet<T, M>::Get(const Value& value) {
    auto lock = std::lock_guard(m_mutex);
    return *m_set.insert(value).first;
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedSet<T, M>::TestAndSet(const Value& value, F&& f) {
    auto lock = std::lock_guard(m_mutex);
    if(m_set.find(value) != m_set.end()) {
      return;
    }
    m_set.insert(value);
    std::forward<F>(f)();
  }

  template<typename T, typename M>
  template<typename V>
  bool SynchronizedSet<T, M>::Insert(V&& value) {
    auto lock = std::lock_guard(m_mutex);
    return m_set.insert(std::forward<V>(value)).second;
  }

  template<typename T, typename M>
  template<typename... Args>
  bool SynchronizedSet<T, M>::Emplace(Args&&... args) {
    auto lock = std::lock_guard(m_mutex);
    return m_set.emplace(std::forward<Args>(args)...).second;
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::Update(const Value& value) {
    auto lock = std::lock_guard(m_mutex);
    auto entry = m_set.insert(value);
    if(entry.second) {
      return;
    }
    m_set.erase(entry.first);
    m_set.insert(value);
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::Clear() {
    auto set = Set();
    {
      auto lock = std::lock_guard(m_mutex);
      set.swap(m_set);
    }
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::Erase(const Value& value) {
    auto lock = std::lock_guard(m_mutex);
    m_set.erase(value);
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::Swap(Set& set) {
    auto lock = std::lock_guard(m_mutex);
    m_set.swap(set);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedSet<T, M>::With(F&& f) {
    auto lock = std::lock_guard(m_mutex);
    return f(m_set);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedSet<T, M>::With(F&& f) const {
    auto lock = std::lock_guard(m_mutex);
    return f(m_set);
  }
}

#endif
