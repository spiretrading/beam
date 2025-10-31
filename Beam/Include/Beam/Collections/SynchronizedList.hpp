#ifndef BEAM_SYNCHRONIZED_LIST_HPP
#define BEAM_SYNCHRONIZED_LIST_HPP
#include <concepts>
#include <deque>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/Algorithm.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

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
       * Constructs a list from a range.
       * @param range The range to copy.
       */
      template<std::ranges::input_range R> requires std::convertible_to<
        std::ranges::range_reference_t<R>, typename T::value_type>
      explicit SynchronizedList(R&& range);

      /**
       * Copies a list.
       * @param list The list to copy.
       */
      template<std::constructible_from<typename T::value_type> U, typename V>
      SynchronizedList(const SynchronizedList<U, V>& list);

      SynchronizedList(const SynchronizedList& list);
      SynchronizedList(SynchronizedList&& list);

      /** Returns a copy of this list. */
      List load() const;

      /**
       * Adds a value at the end of the list.
       * @param value The value to insert.
       */
      template<std::constructible_from<typename T::value_type> V>
      void push_back(V&& value);

      /**
       * Removes and returns the first element from the list.
       * @return The first element, or boost::none if the list is empty.
       */
      boost::optional<Value> pop_front();

      /**
       * Removes a value from the list.
       * @param value The value to remove.
       */
      void erase(const Value& value);

      /**
       * Removes all values matching a predicate.
       * @param f The predicate to match.
       */
      template<std::predicate<typename T::value_type&> F>
      void erase_if(F f);

      /** Performs an action on each element of this list. */
      template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
      void for_each(this Self&& self, F f);

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
      template<typename Self, IsInvocableLike<Self, T> F>
      decltype(auto) with(this Self&& self, F&& f);

    private:
      mutable Mutex m_mutex;
      List m_list;
  };

  template<std::ranges::input_range R>
  SynchronizedList(R&&) ->
    SynchronizedList<std::vector<std::ranges::range_value_t<R>>>;

  /**
   * A SynchronizedList using an std::vector.
   * @tparam V The type of value.
   * @tparam M The type of mutex used to synchronized this container.
   */
  template<typename ValueType, typename M = boost::mutex>
  using SynchronizedVector = SynchronizedList<std::vector<ValueType>, M>;

  /**
   * A SynchronizedList using an std::deque.
   * @tparam V The type of value.
   * @tparam M The type of mutex used to synchronized this container.
   */
  template<typename ValueType, typename M = boost::mutex>
  using SynchronizedDeque = SynchronizedList<std::deque<ValueType>, M>;

  template<typename T, typename M>
  template<std::ranges::input_range R> requires std::convertible_to<
    std::ranges::range_reference_t<R>, typename T::value_type>
  SynchronizedList<T, M>::SynchronizedList(R&& range) {
    m_list.insert(
      m_list.end(), std::ranges::begin(range), std::ranges::end(range));
  }

  template<typename T, typename M>
  template<std::constructible_from<typename T::value_type> U, typename V>
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
  template<std::constructible_from<typename T::value_type> V>
  void SynchronizedList<T, M>::push_back(V&& value) {
    auto lock = boost::lock_guard(m_mutex);
    m_list.push_back(std::forward<V>(value));
  }

  template<typename T, typename M>
  boost::optional<typename SynchronizedList<T, M>::Value>
      SynchronizedList<T, M>::pop_front() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_list.empty()) {
      return boost::none;
    }
    if constexpr(requires { m_list.pop_front(); }) {
      auto value = std::move(m_list.front());
      m_list.pop_front();
      return value;
    } else {
      auto value = std::move(m_list.front());
      m_list.erase(m_list.begin());
      return value;
    }
  }

  template<typename T, typename M>
  void SynchronizedList<T, M>::erase(const Value& value) {
    auto lock = boost::lock_guard(m_mutex);
    std::erase(m_list, value);
  }

  template<typename T, typename M>
  template<std::predicate<typename T::value_type&> F>
  void SynchronizedList<T, M>::erase_if(F f) {
    auto lock = boost::lock_guard(m_mutex);
    std::erase_if(m_list, std::move(f));
  }

  template<typename T, typename M>
  template<typename Self, IsInvocableLike<Self, typename T::value_type> F>
  void SynchronizedList<T, M>::for_each(this Self&& self, F f) {
    auto lock = boost::lock_guard(self.m_mutex);
    if constexpr(std::is_const_v<std::remove_reference_t<Self>> ||
        std::is_lvalue_reference_v<Self>) {
      for(auto& value : self.m_list) {
        f(value);
      }
    } else {
      for(auto&& value : self.m_list) {
        f(std::move(value));
      }
    }
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
  template<typename Self, IsInvocableLike<Self, T> F>
  decltype(auto) SynchronizedList<T, M>::with(this Self&& self, F&& f) {
    auto lock = boost::lock_guard(self.m_mutex);
    return std::forward<F>(f)(std::forward<Self>(self).m_list);
  }

  template<typename T, typename M>
  void swap(SynchronizedList<T, M>& a, SynchronizedList<T, M>& b) {
    a.swap(b);
  }
}

#endif
