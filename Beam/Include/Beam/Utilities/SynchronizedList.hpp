#ifndef BEAM_SYNCHRONIZEDLIST_HPP
#define BEAM_SYNCHRONIZEDLIST_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/Algorithm.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class SynchronizedList
      \brief Wraps a list container allowing for atomic operations to be
             performed on it.
      \tparam ListType The type of list to wrap.
      \tparam MutexType The type of mutex used to synchronized this container.
   */
  template<typename ListType, typename MutexType = boost::mutex>
  class SynchronizedList {
    public:

      //! The type of list being wrapped.
      using List = ListType;

      //! The type of mutex used for synchronization.
      using Mutex = MutexType;

      //! The type of value stored by this list.
      using Value = typename List::value_type;

      //! Constructs an empty list.
      SynchronizedList();

      //! Copies a list.
      /*!
        \param list The list to copy.
      */
      SynchronizedList(const SynchronizedList& list);

      //! Copies a list.
      /*!
        \param list The list to copy.
      */
      template<typename T, typename M>
      SynchronizedList(const SynchronizedList<T, M>& list);

      //! Moves a list.
      /*!
        \param list The list to move.
      */
      SynchronizedList(SynchronizedList&& list);

      //! Returns a copy of this list.
      List Acquire() const;

      //! Adds a value into the end of the list.
      /*!
        \param value The value to insert.
      */
      template<typename ValueForward>
      void PushBack(ValueForward&& value);

      //! Appends a container to the end of the list.
      /*!
        \param container The container to append.
      */
      template<typename Container>
      void Append(const Container& container);

      //! Removes a value from the list.
      /*!
        \param value The value to remove.
      */
      void Remove(const Value& value);

      //! Removes all values matching a predicate.
      /*!
        \param f The predicate to match.
      */
      template<typename F>
      void RemoveIf(F f);

      //! Performs an action on each element of this list.
      template<typename F>
      void ForEach(F f);

      //! Performs an action on each element of this list.
      template<typename F>
      void ForEach(F f) const;

      //! Clears the contents of this list.
      void Clear();

      //! Swaps this list with another.
      /*!
        \param list The list to swap with.
      */
      void Swap(List& list);

      //! Performs a synchronized action with the list.
      /*!
        \param f The action to perform on the list.
      */
      template<typename F>
      decltype(auto) With(F f) {
        boost::lock_guard<Mutex> lock(m_mutex);
        return f(m_list);
      }

      //! Performs a synchronized action with the list.
      /*!
        \param f The action to perform on the list.
      */
      template<typename F>
      decltype(auto) With(F f) const {
        boost::lock_guard<Mutex> lock(m_mutex);
        return f(m_list);
      }

    private:
      mutable Mutex m_mutex;
      List m_list;
  };

  /*! \class SynchronizedVector
      \brief A SynchronizedList using an std::vector.
      \tparam ValueType The type of value.
      \tparam MutexType The type of mutex used to synchronized this container.
   */
  template<typename ValueType, typename MutexType = boost::mutex>
  using SynchronizedVector = SynchronizedList<
    std::vector<ValueType>, MutexType>;

  template<typename ListType, typename MutexType>
  SynchronizedList<ListType, MutexType>::SynchronizedList() {}

  template<typename ListType, typename MutexType>
  SynchronizedList<ListType, MutexType>::SynchronizedList(
      const SynchronizedList& list) {
    boost::lock_guard<Mutex> lock(list.m_mutex);
    m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
  }

  template<typename ListType, typename MutexType>
  template<typename T, typename M>
  SynchronizedList<ListType, MutexType>::SynchronizedList(
      const SynchronizedList<T, M>& list) {
    boost::lock_guard<Mutex> lock(list.m_mutex);
    m_list.insert(m_list.end(), list.m_list.begin(), list.m_list.end());
  }

  template<typename ListType, typename MutexType>
  SynchronizedList<ListType, MutexType>::SynchronizedList(
      SynchronizedList&& list) {
    boost::lock_guard<Mutex> lock(list.m_mutex);
    m_list = std::move(list.m_list);
  }

  template<typename ListType, typename MutexType>
  typename SynchronizedList<ListType, MutexType>::List
      SynchronizedList<ListType, MutexType>::Acquire() const {
    boost::lock_guard<Mutex> lock(m_mutex);
    return m_list;
  }

  template<typename ListType, typename MutexType>
  template<typename ValueForward>
  void SynchronizedList<ListType, MutexType>::PushBack(ValueForward&& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_list.push_back(std::forward<ValueForward>(value));
  }

  template<typename ListType, typename MutexType>
  template<typename Container>
  void SynchronizedList<ListType, MutexType>::Append(
      const Container& container) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_list.insert(m_list.end(), container.begin(), container.end());
  }

  template<typename ListType, typename MutexType>
  void SynchronizedList<ListType, MutexType>::Remove(const Value& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    Beam::RemoveAll(m_list, value);
  }

  template<typename ListType, typename MutexType>
  template<typename F>
  void SynchronizedList<ListType, MutexType>::RemoveIf(F f) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto i = m_list.begin();
    while(i != m_list.end()) {
      if(f(*i)) {
        i = m_list.erase(i);
      } else {
        ++i;
      }
    }
  }

  template<typename ListType, typename MutexType>
  template<typename F>
  void SynchronizedList<ListType, MutexType>::ForEach(F f) {
    boost::lock_guard<Mutex> lock(m_mutex);
    for(auto& entry : m_list) {
      f(entry);
    }
  }

  template<typename ListType, typename MutexType>
  template<typename F>
  void SynchronizedList<ListType, MutexType>::ForEach(F f) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    for(const auto& entry : m_list) {
      f(entry);
    }
  }

  template<typename ListType, typename MutexType>
  void SynchronizedList<ListType, MutexType>::Clear() {
    List list;
    {
      boost::lock_guard<Mutex> lock(m_mutex);
      list.swap(m_list);
    }
  }

  template<typename ListType, typename MutexType>
  void SynchronizedList<ListType, MutexType>::Swap(List& list) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_list.swap(list);
  }
}

#endif
