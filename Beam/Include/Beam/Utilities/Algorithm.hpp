#ifndef BEAM_ALGORITHM_HPP
#define BEAM_ALGORITHM_HPP
#include <algorithm>
#include <deque>
#include <iterator>
#include <map>
#include <ostream>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/optional/optional.hpp>
#include <boost/range.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Merges two ranges together eliminating duplicates.
  /*!
    \param first1 The beginning of the first range to merge.
    \param last1 One past the end of the first range to merge.
    \param first2 The beginning of the second range to merge.
    \param last2 One past the end of the second range to merge.
    \param result The location to store the merged ranges.
    \param predicate The sort/order predicate.
    \return One past the end of the merged range.
  */
  template<typename InputIterator1, typename InputIterator2,
    typename OutputIterator, typename P>
  OutputIterator MergeWithoutDuplicates(InputIterator1 first1,
      InputIterator1 last1, InputIterator2 first2, InputIterator2 last2,
      OutputIterator result, P predicate) {
    while(true) {
      if(first1 == last1) {
        return std::copy(first2, last2, result);
      } else if(first2 == last2) {
        return std::copy(first1, last1, result);
      } else if(predicate(*first1, *first2)) {
        *result = *first1;
        ++first1;
        ++result;
      } else if(predicate(*first2, *first1)) {
        *result = *first2;
        ++first2;
        ++result;
      } else {
        *result = *first1;
        ++first1;
        ++first2;
        ++result;
      }
    }
    return result;
  }

  //! Merges two ranges together eliminating duplicates.
  /*!
    \param first1 The beginning of the first range to merge.
    \param last1 One past the end of the first range to merge.
    \param first2 The beginning of the second range to merge.
    \param last2 One past the end of the second range to merge.
    \param result The location to store the merged ranges.
    \return One past the end of the merged range.
  */
  template<typename InputIterator1, typename InputIterator2,
    typename OutputIterator>
  OutputIterator MergeWithoutDuplicates(InputIterator1 first1,
      InputIterator1 last1, InputIterator2 first2, InputIterator2 last2,
      OutputIterator result) {
    using Type = decltype(*last1);
    return MergeWithoutDuplicates(first1, last1, first2, last2, result,
      std::less<Type>());
  }

  //! Appends one list to another.
  /*!
    \param l The list to modify.
    \param q The list to append.
  */
  template<typename T, typename U>
  void Append(T& l, const U& q) {
    l.insert(l.end(), q.begin(), q.end());
  }

  //! Tests if a container contains a value.
  /*!
    \param container The container to search.
    \param value The value to find in the <i>container</i>.
    \return <code>true</code> iff the <i>value</i> is a member of the
            <i>container</i>.
  */
  template<typename Container, typename Value>
  bool Contains(const Container& container, const Value& value) {
    return std::find(container.begin(), container.end(), value) !=
      container.end();
  }

  //! Does a linear search for an item's lower bound.
  /*!
    \param first An iterator to the beginning of the sequence to search.
    \param last An iterator to the end of the sequence to search.
    \param item The item to search.
    \param comparator The comparator to use.
    \return An iterator to the item's lower bound position.
  */
  template<typename Iterator, typename T, typename Comparator>
  Iterator LinearLowerBound(Iterator first, Iterator last, const T& item,
      Comparator comparator) {
    for(auto i = first; i != last; ++i) {
      if(!comparator(*i, item)) {
        return i;
      }
    }
    return last;
  }

  //! Returns the first element in a range that matches a predicate.
  /*!
    \param range The range to search.
    \param f The predicate to apply.
    \return The first element in the specified <i>range</i> that matches the
            predicate <i>f</i>.
  */
  template<typename Range, typename F>
  typename std::enable_if<!IsManagedPointer<typename Range::value_type>::value,
      boost::optional<typename Range::const_reference>>::type FindIf(
      const Range& range, F&& f) {
    auto i = std::find_if(range.begin(), range.end(), f);
    if(i == range.end()) {
      return boost::none;
    }
    return *i;
  }

  //! Returns the first element in a range that matches a predicate.
  /*!
    \param range The range to search.
    \param f The predicate to apply.
    \return The first element in the specified <i>range</i> that matches the
            predicate <i>f</i>.
  */
  template<typename Range, typename F>
  typename std::enable_if<IsManagedPointer<typename Range::value_type>::value,
      typename Range::value_type>::type FindIf(const Range& range, F&& f) {
    auto i = std::find_if(range.begin(), range.end(),
      [&] (typename Range::const_reference value) {
        return f(*value);
      });
    if(i == range.end()) {
      return nullptr;
    }
    return *i;
  }

  //! Removes the first occurrence of an element in a vector.
  /*!
    \param container The vector to remove the element from.
    \param element The element to remove.
  */
  template<typename T, typename U>
  typename std::enable_if<!IsManagedPointer<typename T::value_type>::value ||
      (IsManagedPointer<typename T::value_type>::value &&
      IsManagedPointer<U>::value), bool>::type RemoveFirst(T& container,
      const U& element) {
    auto itemIterator = std::find(container.begin(), container.end(), element);
    if(itemIterator == container.end()) {
      return false;
    } else if(itemIterator != container.end() - 1) {
      *itemIterator = std::move(container.back());
    }
    container.pop_back();
    return true;
  }

  //! Removes the first occurrence of an element in a vector.
  /*!
    \param container The vector to remove the element from.
    \param element The element to remove.
  */
  template<typename T, typename U>
  typename std::enable_if<IsManagedPointer<typename T::value_type>::value &&
      !IsManagedPointer<U>::value, bool>::type RemoveFirst(T& container,
      const U& element) {
    auto itemIterator = std::find_if(container.begin(), container.end(),
      [&] (const typename T::value_type& i) {
        return i.get() == &element;
      });
    if(itemIterator == container.end()) {
      return false;
    } else if(itemIterator != container.end() - 1) {
      *itemIterator = std::move(container.back());
    }
    container.pop_back();
    return true;
  }

  //! Removes the first occurrence of an element in a vector.
  /*!
    \param container The vector to remove the element from.
    \param element The element to remove.
  */
  template<typename T, typename U>
  typename std::enable_if<!IsManagedPointer<typename T::value_type>::value ||
      (IsManagedPointer<typename T::value_type>::value &&
      IsManagedPointer<U>::value), bool>::type RemoveAll(T& container,
      const U& element) {
    auto size = container.size();
    container.erase(std::remove(container.begin(), container.end(), element),
      container.end());
    return size != container.size();
  }

  //! Removes the first occurrence of an element in a vector.
  /*!
    \param container The vector to remove the element from.
    \param element The element to remove.
  */
  template<typename T, typename U>
  typename std::enable_if<IsManagedPointer<typename T::value_type>::value &&
      !IsManagedPointer<U>::value, bool>::type RemoveAll(T& container,
      const U& element) {
    auto size = container.size();
    container.erase(std::remove_if(container.begin(), container.end(),
      [&] (const typename T::value_type& i) {
        return i.get() == &element;
      }),
      container.end());
    return size != container.size();
  }

  //! Find the value associated with a key within a map.
  /*!
    \param container The container to find the element in.
    \param key The key of the element to find.
    \return The value associated with the <i>key</i>.
  */
  template<typename K, typename V, typename C, typename A, typename U>
  boost::optional<const typename std::unordered_map<K, V, C, A>::mapped_type&>
      Lookup(const std::unordered_map<K, V, C, A>& container, const U& key) {
    auto i = container.find(key);
    if(i == container.end()) {
      return boost::none;
    }
    return i->second;
  }

  //! Find the value associated with a key within a map.
  /*!
    \param container The container to find the element in.
    \param key The key of the element to find.
    \return The value associated with the <i>key</i>.
  */
  template<typename K, typename V, typename C, typename A, typename U>
  boost::optional<typename std::unordered_map<K, V, C, A>::mapped_type&>
      Lookup(std::unordered_map<K, V, C, A>& container, const U& key) {
    auto i = container.find(key);
    if(i == container.end()) {
      return boost::none;
    }
    return i->second;
  }

  //! Returns an iterator to the first occurrence of an element.
  /*!
    \param container The container to find the element in.
    \param element The element to find.
    \return An iterator to the <i>element</i> or to the end if the
            <i>element</i> is not found.
  */
  template<typename T, typename U>
  typename std::enable_if<!IsManagedPointer<typename T::value_type>::value,
      typename T::iterator>::type Find(T& container, const U& element) {
    return std::find(container.begin(), container.end(), element);
  }

  //! Returns an iterator to the first occurrence of an element.
  /*!
    \param container The container to find the element in.
    \param element The element to find.
    \return An iterator to the <i>element</i> or to the end if the
            <i>element</i> is not found.
  */
  template<typename T, typename U>
  typename std::enable_if<IsManagedPointer<typename T::value_type>::value,
      typename T::iterator>::type Find(T& container, const U& element) {
    for(auto i = container.begin(); i != container.end(); ++i) {
      if(i->get() == &element) {
        return i;
      }
    }
    return container.end();
  }

  //! Erases an element in a vector.
  /*!
    \param container The vector to erase the element from.
    \param location An iterator to the location of the element to delete.
  */
  template<typename T>
  typename T::iterator Erase(T& container, typename T::iterator location) {
    auto index = location - container.begin();
    if(location != container.end() - 1) {
      *location = std::move(container.back());
    }
    container.pop_back();
    return container.begin() + index;
  }

  //! Erases all occurrences of an element in a vector.
  /*!
    \param container The vector to erase all occurrences of the element from.
    \param element The element to erase.
  */
  template<typename T, typename U>
  void EraseAll(T& container, const U& element) {
    container.erase(std::remove(container.begin(), container.end(), element),
      container.end());
  }

  //! Finds a value in a map.
  /*!
    \param map The map to perform the lookup on.
    \param key The key to find.
    \return The value retrieved from the <i>map<i> with the specified
            <i>key</i>.
  */
  template<typename T>
  boost::optional<const typename T::mapped_type&> Retrieve(const T& map,
      const typename T::key_type& key) {
    auto valueIterator = map.find(key);
    if(valueIterator == map.end()) {
      return boost::none;
    }
    return valueIterator->second;
  }

  //! Finds a value in a map.
  /*!
    \param map The map to perform the lookup on.
    \param key The key to find.
    \return The value retrieved from the <i>map<i> with the specified
            <i>key</i>.
  */
  template<typename T>
  boost::optional<typename T::mapped_type&> Retrieve(T& map,
      const typename T::key_type& key) {
    auto valueIterator = map.find(key);
    if(valueIterator == map.end()) {
      return boost::none;
    }
    return valueIterator->second;
  }

  //! Finds a value in a map or inserts it.
  /*!
    \param map The map to perform the lookup on.
    \param key The key to find.
    \param f The function to call if the key is not found in the <i>map</i>.
    \return The value retrieved from the <i>map<i> with the specified
            <i>key</i>.
  */
  template<typename T, typename F>
  typename T::mapped_type& GetOrInsert(T& map, const typename T::key_type& key,
      F f) {
    auto valueIterator = map.find(key);
    if(valueIterator == map.end()) {
      valueIterator = map.insert(std::make_pair(key, f())).first;
    }
    return valueIterator->second;
  }

  //! Finds a value in a map or inserts it.
  /*!
    \param map The map to perform the lookup on.
    \param key The key to find.
    \return The value retrieved from the <i>map<i> with the specified
            <i>key</i>.
  */
  template<typename T>
  typename T::mapped_type& GetOrInsert(T& map,
      const typename T::key_type& key) {
    auto valueIterator = map.find(key);
    if(valueIterator == map.end()) {
      valueIterator = map.emplace(std::piecewise_construct,
        std::forward_as_tuple(key), std::forward_as_tuple()).first;
    }
    return valueIterator->second;
  }

  template<typename... T>
  auto Zip(T&&... containers) {
    auto zipBegin = boost::make_zip_iterator(boost::make_tuple(
      std::begin(containers)...));
    auto zipEnd = boost::make_zip_iterator(boost::make_tuple(
      std::end(containers)...));
    return boost::make_iterator_range(zipBegin, zipEnd);
  }
}

//! Prints a vector to an ostream.
/*!
  \param out The stream to print to.
  \param value The vector to print.
  \return <i>out</i> for chaining purposes.
*/
template<typename T>
std::ostream& operator <<(std::ostream& out, const std::vector<T>& value) {
  out << "[";
  for(typename std::vector<T>::const_iterator i = value.begin();
      i != value.end(); ++i) {
    out << *i;
    if(i != value.end() - 1) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

//! Prints a map to an ostream.
/*!
  \param out The stream to print to.
  \param value The vector to print.
  \return <i>out</i> for chaining purposes.
*/
template<typename K, typename V>
std::ostream& operator <<(std::ostream& out, const std::map<K, V>& value) {
  out << "{";
  for(typename std::map<K, V>::const_iterator i = value.begin();
      i != value.end(); ++i) {
    out << i->first << ": " << i->second;
    if(i != value.end() - 1) {
      out << ", ";
    }
  }
  out << "}";
  return out;
}

//! Prints a set to an ostream.
/*!
  \param out The stream to print to.
  \param value The vector to print.
  \return <i>out</i> for chaining purposes.
*/
template<typename T>
std::ostream& operator <<(std::ostream& out, const std::set<T>& value) {
  out << "{";
  for(typename std::set<T>::const_iterator i = value.begin();
      i != value.end(); ++i) {
    out << *i;
    if(i != value.end() - 1) {
      out << ", ";
    }
  }
  out << "}";
  return out;
}

#endif
