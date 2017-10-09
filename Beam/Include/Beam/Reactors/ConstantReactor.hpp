#ifndef BEAM_CONSTANTREACTOR_HPP
#define BEAM_CONSTANTREACTOR_HPP
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
      using Type = GetReactorType<Reactor<T>>;

      //! Constructs a ConstantReactor.
      /*!
        \param value The constant to evaluate to.
      */
      template<typename ValueForward>
      ConstantReactor(ValueForward&& value);

      virtual Type Eval() const;

    private:
      T m_value;
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
      : m_value{std::forward<ValueForward>(value)} {
    this->IncrementSequenceNumber();
    this->SetComplete();
  }

  template<typename T>
  typename ConstantReactor<T>::Type ConstantReactor<T>::Eval() const {
    return m_value;
  }
}
}

#endif
