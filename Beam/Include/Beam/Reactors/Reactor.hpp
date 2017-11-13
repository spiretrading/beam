#ifndef BEAM_REACTOR_HPP
#define BEAM_REACTOR_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class Reactor
      \brief The base class of a Reactor.
      \tparam T The type this Reactor evaluates to.
   */
  template<typename T>
  class Reactor : public BaseReactor {
    public:

      //! The type this Reactor evaluates to.
      using Type = T;

      virtual const std::type_info& GetType() const;

      //! Evaluates this Reactor.
      virtual Type Eval() const = 0;

    protected:

      //! Default constructor.
      Reactor() = default;
  };

  /*! \class ReactorType
      \brief Returns a Reactor's Type.
      \tparam T The Reactor to evaluate.
   */
  template<typename T, typename Enabled = void>
  struct ReactorType {
    using type = typename GetTryDereferenceType<T>::Type;
  };

  //! Invokes a Reactor's Eval method, wrapper any thrown exception in an
  //! Expect.
  /*!
    \param reactor The Reactor to eval.
    \return The result of the eval, capturing any thrown exception.
  */
  template<typename Reactor>
  auto TryEval(const Reactor& reactor) {
    return Try(
      [&] {
        return reactor.Eval();
      });
  }

  //! Helper function for creating a Reactor factory.
  /*!
    \param f The function used to build the Reactor.
  */
  template<typename... Args, typename F>
  auto Factory(F&& f) {
    return [&] (std::shared_ptr<Reactor<Args>>... args) {
      return f(args...);
    };
  }

  template<typename T>
  struct ReactorType<T, typename std::enable_if<
      std::is_same<GetTryDereferenceType<T>, BaseReactor>::value>::type> {
    using type = void;
  };

  template<typename T>
  using GetReactorType = typename ReactorType<T>::type;

  template<typename T>
  const std::type_info& Reactor<T>::GetType() const {
    return typeid(Type);
  }
}
}

#endif
