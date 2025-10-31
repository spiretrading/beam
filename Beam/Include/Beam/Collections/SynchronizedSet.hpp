#ifndef BEAM_SYNCHRONIZED_SET_HPP
#define BEAM_SYNCHRONIZED_SET_HPP
#include <concepts>
#include <unordered_set>
#include <utility>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Wraps a set container allowing for atomic operations to be performed on it.
   * @tparam T The type of set to wrap.
   * @tparam M The type of mutex used for synchronization.
   */
  template<typename T, typename M = boost::mutex>
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
       * Copies a set.
       * @param set The set to copy.
       */
      template<typename U, typename V> requires std::constructible_from<
        typename T::value_type, const typename U::value_type&>
      SynchronizedSet(const SynchronizedSet<U, V>& set);

      SynchronizedSet(const SynchronizedSet& set);
      SynchronizedSet(SynchronizedSet&& set);

      /**
       * Returns <code>true</code> iff this set contains a specified value.
       * @param value The value to find.
       * @return <code>true</code> iff this set contains the <i>value</i>.
       */
      bool contains(const Value& value) const;

      /**
       * Returns a value stored by this set or inserts it if it isn't in the
       * set.
       * @param value The value to search for.
       * @return The value stored in the set.
       */
      Value get(const Value& value);

      /**
       * Inserts a value into the set.
       * @param value The value to insert.
       */
      template<IsConstructibleTo<typename T::value_type> V>
      bool insert(V&& value);

      /**
       * Emplaces a value into the set.
       * @param value The value to insert.
       */
      template<typename... Args> requires
        std::constructible_from<typename T::value_type, Args...>
      bool emplace(Args&&... args);

      /**
       * Finds a value and returns a copy.
       * @param value The value to search for.
       * @return A copy of the found value.
       */
      boost::optional<Value> try_load(const Value& value) const;

      /**
       * Tests if a value is in this set, and if it isn't then inserts it and
       * calls the supplied function.
       * @param value The value to test for.
       * @param f The function to call if the value is not found.
       */
      template<std::invocable F>
      void test_and_set(const Value& value, F&& f);

      /**
       * Finds a value.
       * @param value The value to search for.
       * @return The value contained in this set.
       */
      boost::optional<const Value&> find(const Value& value) const;

      /**
       * Updates a value that may or may not be contained in the set.
       * @param value The updated value.
       */
      void update(const Value& value);

      /** Performs an action on each element of this set. */
      template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
      void for_each(this Self&& self, F f);

      /** Clears the contents of this set. */
      void clear();

      /**
       * Removes a value from this set.
       * @param value The value to remove.
       */
      void erase(const Value& value);

      /**
       * Swaps this set with another.
       * @param set The set to swap with.
       */
      void swap(Set& set);

      /**
       * Performs a synchronized action with the set.
       * @param f The action to perform on the set.
       */
      template<typename Self, IsInvocableLike<Self, T> F>
      decltype(auto) with(this Self&& self, F&& f);

    private:
      mutable Mutex m_mutex;
      Set m_set;
  };

  /**
   * A SynchronizedSet using an std::unordered_set.
   * @param K The set's key.
   * @param M The type of mutex used to synchronized this container.
   */
  template<typename K, typename M = boost::mutex>
  using SynchronizedUnorderedSet = SynchronizedSet<std::unordered_set<K>, M>;

  template<typename T, typename M>
  template<typename U, typename V> requires std::constructible_from<
    typename T::value_type, const typename U::value_type&>
  SynchronizedSet<T, M>::SynchronizedSet(const SynchronizedSet<U, V>& set) {
    auto lock = boost::lock_guard(set.m_mutex);
    m_set.insert(m_set.end(), set.m_set.begin(), set.m_set.end());
  }

  template<typename T, typename M>
  SynchronizedSet<T, M>::SynchronizedSet(const SynchronizedSet& set) {
    auto lock = boost::lock_guard(set.m_mutex);
    m_set = set.m_set;
  }

  template<typename T, typename M>
  SynchronizedSet<T, M>::SynchronizedSet(SynchronizedSet&& set) {
    auto lock = boost::lock_guard(set.m_mutex);
    m_set = std::move(set.m_set);
  }

  template<typename T, typename M>
  bool SynchronizedSet<T, M>::contains(const Value& value) const {
    auto lock = boost::lock_guard(m_mutex);
    return m_set.find(value) != m_set.end();
  }

  template<typename T, typename M>
  typename SynchronizedSet<T, M>::Value
      SynchronizedSet<T, M>::get(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    return *m_set.insert(value).first;
  }

  template<typename T, typename M>
  template<IsConstructibleTo<typename T::value_type> V>
  bool SynchronizedSet<T, M>::insert(V&& value) {
    auto lock = boost::lock_guard(m_mutex);
    return m_set.emplace(std::forward<V>(value)).second;
  }

  template<typename T, typename M>
  template<typename... Args> requires
    std::constructible_from<typename T::value_type, Args...>
  bool SynchronizedSet<T, M>::emplace(Args&&... args) {
    auto lock = boost::lock_guard(m_mutex);
    return m_set.emplace(std::forward<Args>(args)...).second;
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedSet<T, M>::Value>
      SynchronizedSet<T, M>::try_load(const Value& value) const {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_set.find(value);
    if(i == m_set.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T, typename M>
  boost::optional<const typename SynchronizedSet<T, M>::Value&>
      SynchronizedSet<T, M>::find(const Value& value) const {
    auto lock = boost::lock_guard(m_mutex);
    auto i = m_set.find(value);
    if(i == m_set.end()) {
      return boost::none;
    }
    return *i;
  }

  template<typename T, typename M>
  template<std::invocable F>
  void SynchronizedSet<T, M>::test_and_set(const Value& value, F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    if(m_set.find(value) != m_set.end()) {
      return;
    }
    m_set.insert(value);
    std::forward<F>(f)();
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::update(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    auto entry = m_set.insert(value);
    if(entry.second) {
      return;
    }
    m_set.erase(entry.first);
    m_set.insert(value);
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
  void SynchronizedSet<T, M>::for_each(this Self&& self, F f) {
    auto lock = boost::lock_guard(self.m_mutex);
    if constexpr(std::is_const_v<std::remove_reference_t<Self>> ||
        std::is_lvalue_reference_v<Self>) {
      for(auto& value : self.m_set) {
        f(value);
      }
    } else {
      for(auto&& value : self.m_set) {
        f(std::move(value));
      }
    }
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::clear() {
    auto set = Set();
    {
      auto lock = boost::lock_guard(m_mutex);
      set.swap(m_set);
    }
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::erase(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_set.erase(value);
  }

  template<typename T, typename M>
  void SynchronizedSet<T, M>::swap(Set& set) {
    auto lock = boost::lock_guard(m_mutex);
    m_set.swap(set);
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, T> F>
  decltype(auto) SynchronizedSet<T, M>::with(this Self&& self, F&& f) {
    auto lock = boost::lock_guard(self.m_mutex);
    return std::forward<F>(f)(std::forward<Self>(self).m_set);
  }
}

#endif
