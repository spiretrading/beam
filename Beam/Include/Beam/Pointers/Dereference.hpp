#ifndef BEAM_DEREFERENCE_HPP
#define BEAM_DEREFERENCE_HPP
#include <memory>
#include <boost/typeof/typeof.hpp>
#include <boost/utility/declval.hpp>
#include "Beam/Pointers/Pointers.hpp"

namespace Beam {

  /*! \struct IsDereferenceable
      \brief Returns <code>true</code> iff a type can be dereferenced.
   */
  template<typename T>
  struct IsDereferenceable {
    private:
      typedef char YesType;
      typedef struct {
        char a[2];
      } NoType;

      template<typename C>
      static YesType Test(decltype(&C::operator *));

      template<typename C>
      static YesType Test(C*, typename std::enable_if<
        std::is_pointer<C>::value>::type* = nullptr);

      template<typename C>
      static NoType Test(...);

    public:
      static const bool value = sizeof(Test<T>(nullptr)) == sizeof(YesType);
  };

  /*! \struct IsManagedPointer
      \brief Returns <code>true</code> iff a type represents a 'managed'
             pointer.
  */
  template<typename T>
  struct IsManagedPointer {
    static const bool value = false;
  };

  template<typename T, typename D>
  struct IsManagedPointer<std::unique_ptr<T, D>> {
    static const bool value = true;
  };

  template<typename T>
  struct IsManagedPointer<std::shared_ptr<T>> {
    static const bool value = true;
  };

  /*! \struct DereferenceType
      \brief Returns the type resulting from a dereference operation.
   */
  template<typename T>
  struct DereferenceType {
    typedef BOOST_TYPEOF_TPL(*boost::declval<T>()) type;
  };

  template<typename T>
  using GetDereferenceType = typename DereferenceType<T>::type;

  /*! \struct TryDereferenceType
      \brief Returns the type resulting from a dereference operation or the type
             itself if it can't be dereferenced.
   */
  template<typename T>
  struct TryDereferenceType {
    private:
      template<typename U, bool Enabled>
      struct TryDereferenceHelper {
        typedef typename DereferenceType<U>::type type;
      };

      template<typename U>
      struct TryDereferenceHelper<U, false> {
        typedef U type;
      };

    public:
      typedef typename TryDereferenceHelper<typename std::decay<T>::type,
        IsDereferenceable<typename std::decay<T>::type>::value>::type type;
  };

  template<typename T>
  using GetTryDereferenceType = typename TryDereferenceType<T>::type;
}

#endif
