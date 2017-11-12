#ifndef BEAM_THROW_REACTOR_HPP
#define BEAM_THROW_REACTOR_HPP
#include <exception>
#include <utility>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ThrowReactor
      \brief A Reactor that always throws an exception.
   */
  template<typename T>
  class ThrowReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a ThrowReactor.
      /*!
        \param e The exception to throw.
      */
      template<typename E>
      ThrowReactor(E&& e);

      //! Constructs a ThrowReactor.
      /*!
        \param e The exception to throw.
      */
      ThrowReactor(std::exception_ptr e);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::exception_ptr m_exception;
  };

  //! Makes a ThrowReactor.
  /*!
    \param e The exception to throw.
  */
  template<typename T, typename E>
  auto MakeThrowReactor(E&& e) {
    return std::make_shared<ThrowReactor<T>>(std::forward<E>(e));
  }

  template<typename T>
  template<typename E>
  ThrowReactor<T>::ThrowReactor(E&& e)
      : ThrowReactor{std::make_exception_ptr(e)} {}

  template<typename T>
  ThrowReactor<T>::ThrowReactor(std::exception_ptr e)
      : m_exception{std::move(e)} {}

  template<typename T>
  BaseReactor::Update ThrowReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == 0) {
      return BaseReactor::Update::EVAL;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename T>
  typename ThrowReactor<T>::Type ThrowReactor<T>::Eval() const {
    std::rethrow_exception(m_exception);
    throw ReactorUnavailableException{};
  }
}
}

#endif
