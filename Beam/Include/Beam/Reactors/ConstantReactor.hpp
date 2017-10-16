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

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

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

  template<typename T>
  template<typename ValueForward>
  ConstantReactor<T>::ConstantReactor(ValueForward&& value)
      : m_value{std::forward<ValueForward>(value)} {}

  template<typename T>
  bool ConstantReactor<T>::IsComplete() const {
    return true;
  }

  template<typename T>
  BaseReactor::Update ConstantReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == 0) {
      return BaseReactor::Update::EVAL;
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
