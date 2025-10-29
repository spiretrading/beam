#ifndef BEAM_ALGORITHM_HPP
#define BEAM_ALGORITHM_HPP
#include <algorithm>
#include <concepts>
#include <iterator>
#include <map>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range.hpp>
#include "Beam/Pointers/Dereference.hpp"

namespace Beam {

  /**
   * Returns the first element in a range that matches a predicate.
   * @param range The range to search.
   * @param f The predicate to apply.
   * @return The first element in the specified <i>range</i> that matches the
   *         predicate <i>f</i>.
   */
  template<std::ranges::input_range R,
    std::indirect_unary_predicate<std::ranges::iterator_t<R>> F> requires(
      !IsManagedPointer<typename R::value_type>)
  boost::optional<std::ranges::range_reference_t<const R>>
      find_if(const R& range, F&& f) {
    auto i = std::ranges::find_if(range, std::forward<F>(f));
    if(i != std::ranges::end(range)) {
      return *i;
    }
    return boost::none;
  }

  /**
   * Returns the first element in a range that matches a predicate.
   * @param range The range to search.
   * @param f The predicate to apply.
   * @return The first element in the specified <i>range</i> that matches the
   *         predicate <i>f</i>.
   */
  template<std::ranges::input_range R,
    std::predicate<dereference_t<typename R::value_type>> F> requires
      IsManagedPointer<typename R::value_type>
  typename R::value_type find_if(const R& range, F f) {
    auto i =
      std::ranges::find_if(range, [&] (typename R::const_reference value) {
        return f(*value);
      });
    if(i != range.end()) {
      return *i;
    }
    return nullptr;
  }

  /**
   * Removes the first occurrence of an element in a vector.
   * @param container The vector to remove the element from.
   * @param element The element to remove.
   */
  template<std::ranges::input_range T, typename U> requires(
    !IsManagedPointer<typename T::value_type> ||
    (IsManagedPointer<typename T::value_type> && IsManagedPointer<U>))
  bool remove_first(T& container, const U& element) {
    auto i = std::find(container.begin(), container.end(), element);
    if(i == container.end()) {
      return false;
    } else if(i != container.end() - 1) {
      *i = std::move(container.back());
    }
    container.pop_back();
    return true;
  }

  /**
   * Removes the first occurrence of an element in a vector.
   * @param container The vector to remove the element from.
   * @param element The element to remove.
   */
  template<std::ranges::input_range T, typename U> requires(
    IsManagedPointer<typename T::value_type> && !IsManagedPointer<U>)
  bool remove_first(T& container, const U& element) {
    auto i = std::find_if(container.begin(), container.end(),
      [&] (const typename T::value_type& i) {
        return i.get() == &element;
      });
    if(i == container.end()) {
      return false;
    } else if(i != container.end() - 1) {
      *i = std::move(container.back());
    }
    container.pop_back();
    return true;
  }

  /**
   * Find the value associated with a key within a map.
   * @param container The container to find the element in.
   * @param key The key of the element to find.
   * @return The value associated with the <i>key</i>.
   */
  template<typename K, typename V, typename C, typename A, typename U>
  boost::optional<const typename std::unordered_map<K, V, C, A>::mapped_type&>
      lookup(const std::unordered_map<K, V, C, A>& container, const U& key) {
    auto i = container.find(key);
    if(i == container.end()) {
      return boost::none;
    }
    return i->second;
  }

  /**
   * Find the value associated with a key within a map.
   * @param container The container to find the element in.
   * @param key The key of the element to find.
   * @return The value associated with the <i>key</i>.
   */
  template<typename K, typename V, typename C, typename A, typename U>
  boost::optional<typename std::unordered_map<K, V, C, A>::mapped_type&>
      lookup(std::unordered_map<K, V, C, A>& container, const U& key) {
    auto i = container.find(key);
    if(i == container.end()) {
      return boost::none;
    }
    return i->second;
  }

  /**
   * Erases an element in a vector where order doesn't matter.
   * @param container The vector to erase the element from.
   * @param location An iterator to the location of the element to delete.
   */
  template<typename T, typename A>
  auto swap_erase(std::vector<T, A>& container,
      typename std::vector<T, A>::iterator location) {
    auto index = location - container.begin();
    if(location != container.end() - 1) {
      *location = std::move(container.back());
    }
    container.pop_back();
    return container.begin() + index;
  }

  /**
   * Finds a value in a map or inserts it.
   * @param map The map to perform the lookup on.
   * @param key The key to find.
   * @param f The function to call if the key is not found in the <i>map</i>.
   * @return The value retrieved from the <i>map<i> with the specified
   *         <i>key</i>.
   */
  template<typename M, typename K, std::invocable F> requires
    std::constructible_from<
      typename M::mapped_type, std::invoke_result_t<F&>> ||
    std::assignable_from<typename M::mapped_type&, std::invoke_result_t<F&>>
  auto& get_or_insert(M& map, K&& key, F&& f) {
    auto& probe = key;
    auto i = map.find(probe);
    if(i != map.end()) {
      return i->second;
    }
    return map.emplace(
      std::forward<K>(key), std::forward<F>(f)()).first->second;
  }

  /**
   * Finds a value in a map or inserts it.
   * @param map The map to perform the lookup on.
   * @param key The key to find.
   * @return The value retrieved from the <i>map<i> with the specified
   *         <i>key</i>.
   */
  template<typename T>
  auto& get_or_insert(T& map, const typename T::key_type& key) {
    auto i = map.find(key);
    if(i == map.end()) {
      i = map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key), std::forward_as_tuple()).first;
    }
    return i->second;
  }

  /**
   * Zips multiple containers together.
   * @param containers The containers to zip together.
   * @return A range of tuples where each tuple contains one element from
   *         each of the specified <i>containers</i>.
   */
  template<typename... T>
  auto zip(T&&... containers) {
    auto begin =
      boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
    auto end =
      boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
    return boost::make_iterator_range(begin, end);
  }
}

#endif
