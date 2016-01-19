#ifndef BEAM_ACTIVEOBJECT_HPP
#define BEAM_ACTIVEOBJECT_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Active
      \brief Encapsulates an object whose updates result in newly created
             values.
      \tparam T The type of value to store.
   */
  template<typename T>
  class Active : private boost::noncopyable {
    public:

      //! The type of value to store.
      using Type = T;

      //! Constructs an Active.
      /*!
        \param args The parameters forwarded to the value's constructor.
      */
      template<typename... U>
      Active(U&&... args);

      //! Returns the currently stored value.
      std::shared_ptr<const Type> Load() const;

      //! Returns the currently stored value.
      std::shared_ptr<Type> Load();

      //! Updates the value stored.
      /*!
        \param value The new value to store.
      */
      template<typename... U>
      void Update(U&&... value);

    private:
      mutable boost::mutex m_mutex;
      std::shared_ptr<Type> m_value;
  };

  template<typename T>
  template<typename... U>
  Active<T>::Active(U&&... args)
      : m_value{std::make_shared<Type>(std::forward<U>(args)...)} {}

  template<typename T>
  std::shared_ptr<const typename Active<T>::Type> Active<T>::Load() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return m_value;
  }

  template<typename T>
  std::shared_ptr<typename Active<T>::Type> Active<T>::Load() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return m_value;
  }

  template<typename T>
  template<typename... U>
  void Active<T>::Update(U&&... value) {
    auto update = std::make_shared<Type>(std::forward<U>(value)...);
    boost::lock_guard<boost::mutex> lock{m_mutex};
    update.swap(m_value);
  }
}

#endif
