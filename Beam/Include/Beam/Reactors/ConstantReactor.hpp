#ifndef BEAM_CONSTANT_REACTOR_HPP
#define BEAM_CONSTANT_REACTOR_HPP
#include <memory>
#include <type_traits>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ConstantReactor
      \brief Evaluates to a constant.
   */
  template<typename T>
  class ConstantReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a ConstantReactor.
      /*!
        \param value The constant to evaluate to.
      */
      template<typename ValueForward>
      ConstantReactor(ValueForward&& value);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      Type m_value;
  };

  //! Makes a ConstantReactor.
  /*!
    \param value The constant to evaluate to.
  */
  template<typename T>
  auto MakeConstantReactor(T&& value) {
    return std::make_shared<ConstantReactor<typename std::decay<T>::type>>(
      std::forward<T>(value));
  }

namespace Details {
  template<typename T>
  struct LiftHelper {
    template<typename U>
    auto operator ()(U&& value) const {
      return MakeConstantReactor(std::forward<U>(value));
    }
  };

  template<typename T>
  struct LiftHelper<std::shared_ptr<T>> {
    template<typename U>
    decltype(auto) operator ()(U&& value) const {
      return std::forward<U>(value);
    }
  };
}

  //! Lifts a constant value to a Reactor unless the parameter is already a
  //! Reactor type.
  /*!
    \param value The value to lift.
  */
  template<typename T>
  decltype(auto) Lift(T&& value) {
    return Details::LiftHelper<typename std::decay<T>::type>{}(
      std::forward<T>(value));
  }

  template<typename T>
  template<typename ValueForward>
  ConstantReactor<T>::ConstantReactor(ValueForward&& value)
      : m_value{std::forward<ValueForward>(value)} {}

  template<typename T>
  BaseReactor::Update ConstantReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == 0) {
      return BaseReactor::Update::COMPLETE_WITH_EVAL;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename T>
  typename ConstantReactor<T>::Type ConstantReactor<T>::Eval() const {
    return m_value;
  }
}
}

#endif
