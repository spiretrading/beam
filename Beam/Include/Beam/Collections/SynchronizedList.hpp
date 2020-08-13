#ifndef BEAM_SYNCHRONIZED_LIST_HPP
#define BEAM_SYNCHRONIZED_LIST_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {

  /**
   * Wraps a list container allowing for atomic operations to be performed on
   * it.
   * @param <T> The type of list to wrap.
   * @param <M> The type of mutex used to synchronized this container.
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
      SynchronizedList(const SynchronizedList& list);

      /**
       * Copies a list.
       * @param list The list to copy.
       */
      template<typename U, typename V>
      SynchronizedList(const SynchronizedList<U, V>& list);

      /**
       * Moves a list.
       * @param list The list to move.
       */
      SynchronizedList(SynchronizedList&& list);

      /** Returns a copy of this list. */
      List Acquire() const;

      /**
       * Adds a value into the end of the list.
       * @param value The value to insert.
       */
      template<typename ValueForward>
      void PushBack(ValueForward&& value);

      /**
       * Appends a container to the end of the list.
       * @param container The container to append.
       */
      template<typename Container>
      void Append(const Container& container);

      /**
       * Removes a value from the list.
       * @param value The value to remove.
       */
      void Remove(const Value& value);

      /**
       * Removes all values matching a predicate.
       * @param f The predicate to match.
       */
      template<typename F>
      void RemoveIf(F&& f);

      /** Performs an action on each element of this list. */
      template<typename F>
      void ForEach(F&& f);

      /** Performs an action on each element of this list. */
      template<typename F>
      void ForEach(F&& f) const;

      /** Clears the contents of this list. */
      void Clear();

      /**
       * Swaps this list with another.
       * @param list The list to swap with.
       */
      void Swap(List& list);

      /**
       * Performs a synchronized action with the list.
       * @param f The action to perform on the list.
       */
      template<typename F>
      decltype(auto) With(F&& f);

      /**
       * Performs a synchronized action with the list.
       * @param f The action to perform on the list.
       */
      template<typename F>
      decltype(auto) With(F&& f) const;

    private:
      mutable Mutex m_mutex;
      List m_list;
  };

  /**
   * A SynchronizedList using an std::vector.
   * @param <V> The type of value.
   * @param <M> The type of mutex used to synchronized this container.
   */
  template<typename ValueType, typename M = boost::mutex>
  using SynchronizedVector = SynchronizedList<std::vector<ValueType>, M>;

  template<typename T, typename M>
  SynchronizedList<T, M>::SynchronizedList(const SynchronizedList& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
  }

  template<typename T, typename M>
  template<typename U, typename V>
  SynchronizedList<T, M>::SynchronizedList(const SynchronizedList<U, V>& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
  }

  template<typename T, typename M>
  SynchronizedList<T, M>::SynchronizedList(SynchronizedList&& list) {
    auto lock = boost::lock_guard(list.m_mutex);
    m_list = std::move(list.m_list);
  }

  template<typename T, typename M>
  typename SynchronizedList<T, M>::List
      SynchronizedList<T, M>::Acquire() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_list;
  }

  template<typename T, typename M>
  template<typename ValueForward>
  void SynchronizedList<T, M>::PushBack(ValueForward&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.push_back(std::forward<ValueForward>(value));
  }

  template<typename T, typename M>
  template<typename Container>
  void SynchronizedList<T, M>::Append(const Container& container) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.insert(m_list.end(), container.begin(), container.end());
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::Remove(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    Beam::RemoveAll(m_list, value);
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedList<T, M>::RemoveIf(F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.erase(std::remove_if(m_list.begin(), m_list.end(),
      std::forward<F>(f)), m_list.end());
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedList<T, M>::ForEach(F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    std::for_each(m_list.begin(), m_list.end(), std::forward<F>(f));
  }

  template<typename T, typename M>
  template<typename F>
  void SynchronizedList<T, M>::ForEach(F&& f) const {
    auto lock = boost::lock_guard(m_mutex);
    std::for_each(m_list.begin(), m_list.end(), std::forward<F>(f));
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::Clear() {
    auto list = List();
    {
      auto lock = boost::lock_guard(m_mutex);
      list.swap(m_list);
    }
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::Swap(List& list) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.swap(list);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedList<T, M>::With(F&& f) {
    auto lock = boost::lock_guard(m_mutex);
    return f(m_list);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) SynchronizedList<T, M>::With(F&& f) const {
    auto lock = boost::lock_guard(m_mutex);
    return f(m_list);
  }
}

#endif
