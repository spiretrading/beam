#ifndef BEAM_STORAGETYPE_HPP
#define BEAM_STORAGETYPE_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Utilities/NullType.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  template<typename T, typename Enabled = void>
  struct VoidReturnImplementation {
    template<typename Q>
    decltype(auto) operator ()(Q&& result) {
      return std::forward<Q>(result);
    }
  };

  template<typename T>
  struct VoidReturnImplementation<T,
      typename std::enable_if<std::is_same<T, NullType>::value>::type> {
    template<typename Q>
    NullType operator ()(Q&& result) {
      return NullType();
    }
  };
}

  /*! \struct StorageType
      \brief Evaluates to NullType for void parameters, otherwise evaluates to
             its parameter.  Used to unify writing templates for void and
             non-void types.
   */
  template<typename T>
  struct StorageType {
    using type = T;
  };

  template<typename T>
  using GetStorageType = typename StorageType<T>::type;

  template<>
  struct StorageType<void> {
    using type = NullType;
  };

  //! Returns a NullType for void template parameters or else the type's normal
  //! evaluation.
  template<typename T>
  decltype(auto) VoidReturn(T&& result) {
    return Details::VoidReturnImplementation<
      typename std::remove_reference<T>::type>()(std::forward<T>(result));
  }
}

#endif
