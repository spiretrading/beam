#ifndef BEAM_CLONEPTR_HPP
#define BEAM_CLONEPTR_HPP
#include <ostream>
#include <memory>
#include <type_traits>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Pointers.hpp"
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {

  /*! \struct VirtualCloner
      \brief Clones an object by invoking its Clone method.
   */
  struct VirtualCloner {

    template<typename T>
    T* operator()(const T& object) const;
  };

  /*! \class ClonePtr
      \brief Wraps a value by cloning it when copying or assigning.
      \tparam T The type of value to wrap.
      \tparam ClonerType Specifies how to clone objects.
   */
  template<typename T, typename ClonerType>
  class ClonePtr {
    public:

      //! The type of value to wrap.
      using Type = T;

      //! Specifies how to clone objects.
      using Cloner = ClonerType;

      //! The type of value to wrap.
      using element_type = Type;

      //! Constructs a ClonePtr.
      ClonePtr();

      //! Copies a ClonePtr by cloning it.
      /*!
        \param object The ClonePtr to clone.
      */
      ClonePtr(const ClonePtr& object);

      //! Copies a ClonePtr by cloning it.
      /*!
        \param object The ClonePtr to clone.
      */
      template<typename U, typename UC>
      ClonePtr(const ClonePtr<U, UC>& object);

      //! Clones an object into a ClonePtr.
      /*!
        \param object The object to clone.
      */
      template<typename U>
      ClonePtr(const U& object);

      //! Assigns a ClonePtr by cloning it.
      /*!
        \param object The ClonePtr to clone.
      */
      ClonePtr& operator =(const ClonePtr& object);

      //! Assigns a ClonePtr by cloning it.
      /*!
        \param object The ClonePtr to clone.
      */
      template<typename U, typename UC>
      ClonePtr& operator =(const ClonePtr<U, UC>& object);

      //! Converts this object to another type.
      template<typename U>
      const U& StaticCast() const;

      //! Converts this object to another type.
      template<typename U>
      U& StaticCast();

      //! Converts this object to another type.
      template<typename U>
      boost::optional<const U&> DynamicCast() const;

      //! Converts this object to another type.
      template<typename U>
      boost::optional<U&> DynamicCast();

      //! Returns a reference to the value.
      Type& operator *() const;

      //! Returns a pointer to the value.
      Type* operator ->() const;

    private:
      template<typename, typename> friend struct Serialization::Receive;
      template<typename, typename> friend struct Serialization::Send;
      std::unique_ptr<Type> m_object;

      ClonePtr(Serialization::ReceiveBuilder);
  };

  template<typename T, typename TC>
  std::ostream& operator <<(std::ostream& out, const ClonePtr<T, TC>& object) {
    return out << *object;
  }

  //! Tests for equality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>*lhs</i> has the same runtime type as
            <i>*rhs</i> and <i>*lhs</i> == <i>*rhs</i>.
  */
  template<typename T, typename TC, typename U, typename UC>
  bool operator ==(const ClonePtr<T, TC>& lhs, const ClonePtr<U, UC>& rhs) {
    return lhs == *rhs;
  }

  //! Tests for equality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>*lhs</i> has the same runtime type as
            <i>*rhs</i> and <i>*lhs</i> == <i>*rhs</i>.
  */
  template<typename T, typename TC, typename U>
  typename std::enable_if<std::is_base_of<T, U>::value ||
      std::is_same<T, U>::value, bool>::type operator ==(
      const ClonePtr<T, TC>& lhs, const U& rhs) {
    if(!std::is_same<T, U>::value) {
      if(typeid(*lhs) != typeid(rhs)) {
        return false;
      }
    }
    return static_cast<const U&>(*lhs) == rhs;
  }

  //! Tests for equality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>*lhs</i> has the same runtime type as
            <i>*rhs</i> and <i>*lhs</i> == <i>*rhs</i>.
  */
  template<typename T, typename TC, typename U>
  typename std::enable_if<std::is_base_of<U, T>::value &&
      !std::is_same<U, T>::value, bool>::type operator ==(
      const ClonePtr<T, TC>& lhs, const U& rhs) {
    if(typeid(*lhs) != typeid(rhs)) {
      return false;
    }
    return *lhs == static_cast<const T&>(rhs);
  }

  //! Tests for equality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>*lhs</i> has the same runtime type as
            <i>*rhs</i> and <i>*lhs</i> == <i>*rhs</i>.
  */
  template<typename T, typename TC, typename U>
  bool operator ==(const U& rhs, const ClonePtr<T, TC>& lhs) {
    return lhs == rhs;
  }

  //! Tests two ClonePtrs for inequality by comparing their values.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
  */
  template<typename T, typename TC, typename U, typename UC>
  bool operator !=(const ClonePtr<T, TC>& lhs, const ClonePtr<U, UC>& rhs) {
    return !(lhs == rhs);
  }

  //! Tests for inequality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
  */
  template<typename T, typename TC, typename U>
  bool operator !=(const ClonePtr<T, TC>& lhs, const U& rhs) {
    return !(lhs == rhs);
  }

  //! Tests for inequality using a value comparison.
  /*!
    \param lhs The left hand side of the equality.
    \param rhs The right hand side of the equality.
    \return <code>true</code> iff <i>lhs</i> is not equal to <i>rhs</i>.
  */
  template<typename T, typename TC, typename U>
  bool operator !=(const U& lhs, const ClonePtr<T, TC>& rhs) {
    return !(lhs == rhs);
  }

  /*! \class Cloneable
      \brief Base class for an object that can be cloned polymorphically.
   */
  class Cloneable {
    public:
      virtual ~Cloneable() = default;

    protected:

      //! Returns a copy of this object.
      virtual void* Clone() const = 0;

    private:
      template<typename, typename> friend class ClonePtr;
      friend struct VirtualCloner;
  };

  /*! \class CloneableMixin
      \brief Mixin class that implements the Cloneable interface.
      \tparam T The class to provide the mixin to.
   */
  template<typename T>
  class CloneableMixin : public virtual Cloneable {
    protected:
      virtual void* Clone() const;

    private:
      template<typename, typename> friend class ClonePtr;
      friend struct VirtualCloner;
  };

  template<typename T>
  T* VirtualCloner::operator()(const T& object) const {
    return static_cast<T*>(object.Clone());
  }

  template<typename T, typename ClonerType>
  ClonePtr<T, ClonerType>::ClonePtr() {}

  template<typename T, typename ClonerType>
  ClonePtr<T, ClonerType>::ClonePtr(const ClonePtr& object)
      : m_object(Cloner()(*object)) {}

  template<typename T, typename ClonerType>
  template<typename U, typename UC>
  ClonePtr<T, ClonerType>::ClonePtr(const ClonePtr<U, UC>& object)
      : m_object(static_cast<T*>(
          typename ClonePtr<U, UC>::Cloner()(*object))) {}

  template<typename T, typename ClonerType>
  template<typename U>
  ClonePtr<T, ClonerType>::ClonePtr(const U& object)
      : m_object(static_cast<T*>(Cloner()(object))) {}

  template<typename T, typename ClonerType>
  ClonePtr<T, ClonerType>& ClonePtr<T, ClonerType>::operator =(
      const ClonePtr& object) {
    if(this == &object) {
      return *this;
    }
    m_object.reset(Cloner()(*object));
    return *this;
  }

  template<typename T, typename ClonerType>
  template<typename U, typename UC>
  ClonePtr<T, ClonerType>& ClonePtr<T, ClonerType>::operator =(
      const ClonePtr<U, UC>& object) {
    m_object.reset(static_cast<T*>(
      typename ClonePtr<U, UC>::Cloner()(*object)));
    return *this;
  }

  template<typename T, typename ClonerType>
  template<typename U>
  const U& ClonePtr<T, ClonerType>::StaticCast() const {
    return static_cast<const U&>(*m_object);
  }

  template<typename T, typename ClonerType>
  template<typename U>
  U& ClonePtr<T, ClonerType>::StaticCast() {
    return static_cast<U&>(*m_object);
  }

  template<typename T, typename ClonerType>
  template<typename U>
  boost::optional<const U&> ClonePtr<T, ClonerType>::DynamicCast() const {
    try {
      return dynamic_cast<const U&>(*m_object);
    } catch(const std::exception&) {
      return boost::none;
    }
  }

  template<typename T, typename ClonerType>
  template<typename U>
  boost::optional<U&> ClonePtr<T, ClonerType>::DynamicCast() {
    try {
      return dynamic_cast<U&>(*m_object);
    } catch(const std::exception&) {
      return boost::none;
    }
  }

  template<typename T, typename ClonerType>
  typename ClonePtr<T, ClonerType>::Type& ClonePtr<T, ClonerType>::
      operator *() const {
    return *m_object;
  }

  template<typename T, typename ClonerType>
  typename ClonePtr<T, ClonerType>::Type* ClonePtr<T, ClonerType>::
      operator ->() const {
    return m_object.get();
  }

  template<typename T, typename ClonerType>
  ClonePtr<T, ClonerType>::ClonePtr(Serialization::ReceiveBuilder) {}

  template<typename T>
  void* CloneableMixin<T>::Clone() const {
    return new T(static_cast<const T&>(*this));
  }
}

namespace Beam {
namespace Serialization {
  template<typename T, typename TC>
  struct IsDefaultConstructable<ClonePtr<T, TC>> : std::false_type {};

  template<typename T, typename TC>
  struct IsStructure<ClonePtr<T, TC>> : std::false_type {};

  template<typename T, typename TC>
  struct Send<ClonePtr<T, TC>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const ClonePtr<T, TC>& value) const {
      ShuttleNonNull(shuttle, name, value.m_object);
    }
  };

  template<typename T, typename TC>
  struct Receive<ClonePtr<T, TC>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        ClonePtr<T, TC>& value) const {
      ShuttleNonNull(shuttle, name, value.m_object);
    }
  };
}
}

#endif
