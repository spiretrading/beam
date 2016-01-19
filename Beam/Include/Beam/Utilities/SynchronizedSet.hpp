#ifndef BEAM_SYNCHRONIZEDSET_HPP
#define BEAM_SYNCHRONIZEDSET_HPP
#include <unordered_set>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class SynchronizedSet
      \brief Wraps a set container allowing for atomic operations to be
             performed on it.
      \tparam SetType The type of set to wrap.
      \tparam MutexType The type of mutex used for synchronization.
   */
  template<typename SetType, typename MutexType = boost::mutex>
  class SynchronizedSet : public boost::noncopyable {
    public:

      //! The type of set being wrapped.
      typedef SetType Set;

      //! The type of mutex used for synchronization.
      typedef MutexType Mutex;

      //! The type of value stored by this set.
      typedef typename Set::value_type Value;

      //! Constructs an empty set.
      SynchronizedSet();

      //! Returns <code>true</code> iff this set contains a specified value.
      /*!
        \param value The value to find.
        \return <code>true</code> iff this set contains the <i>value</i>.
      */
      bool Contains(const Value& value) const;

      //! Returns a value stored by this set or inserts it if it isn't in the
      //! set.
      /*!
        \param value The value to search for.
        \return The value stored in the set.
      */
      Value Get(const Value& value);

      //! Tests if a value is in this set, and if it isn't then inserts it and
      //! calls the supplied function.
      /*!
        \param value The value to test for.
        \param f The function to call if the value is not found.
      */
      template<typename F>
      void TestAndSet(const Value& value, F f);

      //! Inserts a value into the set.
      /*!
        \param value The value to insert.
      */
      template<typename ValueForward>
      bool Insert(ValueForward&& value);

      //! Updates a value that may or may not be contained in the set.
      /*!
        \param value The updated value.
      */
      void Update(const Value& value);

      //! Clears the contents of this set.
      void Clear();

      //! Removes a value from this set.
      /*!
        \param value The value to remove.
      */
      void Erase(const Value& value);

      //! Swaps this set with another.
      /*!
        \param set The set to swap with.
      */
      void Swap(Set& set);

      //! Performs a synchronized action with the set.
      /*!
        \param f The action to perform on the set.
      */
      template<typename F>
      void With(F f);

      //! Performs a synchronized action with the set.
      /*!
        \param f The action to perform on the set.
      */
      template<typename F>
      void With(F f) const;

    private:
      mutable Mutex m_mutex;
      Set m_set;
  };

  /*! \class SynchronizedUnorderedSet
      \brief A SynchronizedSet using an std::unordered_set.
      \tparam KeyType The set's key.
      \tparam MutexType The type of mutex used to synchronized this container.
   */
  template<typename KeyType, typename MutexType = boost::mutex>
  using SynchronizedUnorderedSet = SynchronizedSet<std::unordered_set<KeyType>,
    MutexType>;

  template<typename SetType, typename MutexType>
  SynchronizedSet<SetType, MutexType>::SynchronizedSet() {}

  template<typename SetType, typename MutexType>
  bool SynchronizedSet<SetType, MutexType>::Contains(const Value& value) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    return m_set.find(value) != m_set.end();
  }

  template<typename SetType, typename MutexType>
  typename SynchronizedSet<SetType, MutexType>::Value
      SynchronizedSet<SetType, MutexType>::Get(const Value& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    return *m_set.insert(value).first;
  }

  template<typename SetType, typename MutexType>
  template<typename F>
  void SynchronizedSet<SetType, MutexType>::TestAndSet(
      const Value& value, F f) {
    boost::lock_guard<Mutex> lock(m_mutex);
    if(m_set.find(value) != m_set.end()) {
      return;
    }
    m_set.insert(value);
    f();
  }

  template<typename SetType, typename MutexType>
  template<typename ValueForward>
  bool SynchronizedSet<SetType, MutexType>::Insert(ValueForward&& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    bool inserted = m_set.insert(std::forward<ValueForward>(value)).second;
    return inserted;
  }

  template<typename SetType, typename MutexType>
  void SynchronizedSet<SetType, MutexType>::Update(const Value& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    auto entry = m_set.insert(value);
    if(entry.second) {
      return;
    }
    m_set.erase(entry.first);
    m_set.insert(value);
  }

  template<typename SetType, typename MutexType>
  void SynchronizedSet<SetType, MutexType>::Clear() {
    Set set;
    {
      boost::lock_guard<Mutex> lock(m_mutex);
      set.swap(m_set);
    }
  }

  template<typename SetType, typename MutexType>
  void SynchronizedSet<SetType, MutexType>::Erase(const Value& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_set.erase(value);
  }

  template<typename SetType, typename MutexType>
  void SynchronizedSet<SetType, MutexType>::Swap(Set& set) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_set.swap(set);
  }

  template<typename SetType, typename MutexType>
  template<typename F>
  void SynchronizedSet<SetType, MutexType>::With(F f) {
    boost::lock_guard<Mutex> lock(m_mutex);
    f(m_set);
  }

  template<typename SetType, typename MutexType>
  template<typename F>
  void SynchronizedSet<SetType, MutexType>::With(F f) const {
    boost::lock_guard<Mutex> lock(m_mutex);
    f(m_set);
  }
}

#endif
