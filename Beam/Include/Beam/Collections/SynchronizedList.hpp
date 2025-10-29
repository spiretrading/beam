#ifndef BEAM_SYNCHRONIZED_LIST_HPP
#define BEAM_SYNCHRONIZED_LIST_HPP
#include <concepts>
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {

  /**
   * Wraps a list container allowing for atomic operations to be performed on
   * it.
   * @tparam T The type of list to wrap.
   * @tparam M The type of mutex used to synchronized this container.
   */
  template<typename T, typename M = boost::mutex>
  class SynchronizedList {
    public:

      /** The type of list being wrapped. */
      using List = T;

      /** The type of mutex used for synchronization. */
      using Mutex = M;

      /** The type of value stored by this list. */
      using Value = typename List::value_type;

      /** Constructs an empty list. */
      SynchronizedList() = default;

      /**
       * Copies a list.
       * @param list The list to copy.
       */
      template<typename U, typename V>
      SynchronizedList(const SynchronizedList<U, V>& list);

      SynchronizedList(const SynchronizedList& list);
      SynchronizedList(SynchronizedList&& list);

      /** Returns a copy of this list. */
      List load() const;

      /**
       * Adds a value at the end of the list.
       * @param value The value to insert.
       */
      template<typename V>
      void push_back(V&& value);

      /**
       * Appends a container to the end of the list.
       * @param container The container to append.
       */
      template<typename C>
      void append(const C& container);

      /**
       * Removes a value from the list.
       * @param value The value to remove.
       */
      void erase(const Value& value);

      /**
       * Removes all values matching a predicate.
       * @param f The predicate to match.
       */
      template<std::predicate<const Value&> F>
      void erase_if(F f) {
        auto lock = boost::lock_guard(m_mutex);
        std::erase_if(m_list, std::move(f));
      }

      /** Performs an action on each element of this list. */
      template<typename F>
      void for_each(F f);

      /** Performs an action on each element of this list. */
      template<typename F>
      void for_each(F f) const;

      /** Clears the contents of this list. */
      void clear();

      /**
       * Swaps this list with another.
       * @param list The list to swap with.
       */
      void swap(List& list);

      /**
       * Performs a synchronized action with the list.
       * @param f The action to perform on the list.
       */
      template<typename F>
      decltype(auto) with(F&& f);

      /**
       * Performs a synchronized action with the list.
       * @param f The action to perform on the list.
       */
      template<typename F>
      decltype(auto) with(F&& f) const;

    private:
      mutable Mutex m_mutex;
      List m_list;
  };

  /**
   * A SynchronizedList using an std::vector.
   * @tparam V The type of value.
   * @tparam M The type of mutex used to synchronized this container.
   */
  template<typename ValueType, typename M = boost::mutex>
  using SynchronizedVector = SynchronizedList<std::vector<ValueType>, M>;

  template<typename T, typename M>
  template<typename U, typename V>
  SynchronizedList<T, M>::SynchronizedList(const SynchronizedList<U, V>& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
  }

  template<typename T, typename M>
  SynchronizedList<T, M>::SynchronizedList(const SynchronizedList& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list = list.m_list;
  }

  template<typename T, typename M>
  SynchronizedList<T, M>::SynchronizedList(SynchronizedList&& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list = std::move(list.m_list);
  }

  template<typename T, typename M>
  typename SynchronizedList<T, M>::List SynchronizedList<T, M>::load() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_list;
  }

  template<typename T, typename M>
  template<typename V>
  void SynchronizedList<T, M>::push_back(V&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.push_back(std::forward<V>(value));
  }

  template<typename T, typename M>
  template<typename C>
  void SynchronizedList<T, M>::append(const C& container) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.insert(m_list.end(), container.begin(), container.end());
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::erase(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    std::erase(m_list, value);
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedList<T, M>::for_each(F f) {
    auto lock = boost::lock_guard(m_mutex);
    std::for_each(m_list.begin(), m_list.end(), std::move(f));
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedList<T, M>::for_each(F f) const {
    auto lock = boost::lock_guard(m_mutex);
    std::for_each(m_list.begin(), m_list.end(), std::move(f));
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::clear() {
    auto list = List();
    {
      auto lock = boost::lock_guard(m_mutex);
      list.swap(m_list);
    }
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::swap(List& list) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.swap(list);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedList<T, M>::with(F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    return std::forward<F>(f)(m_list);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedList<T, M>::with(F&& f) const {
    auto lock = boost::lock_guard(m_mutex);
    return std::forward<F>(f)(m_list);
  }

  template<typename T, typename M>
  void swap(SynchronizedList<T, M>& a, SynchronizedList<T, M>& b) {
    a.swap(b);
  }
}

#endif
