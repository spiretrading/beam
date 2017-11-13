#ifndef BEAM_NONE_REACTOR_HPP
#define BEAM_NONE_REACTOR_HPP
#include <memory>
#include <boost/throw_exception.hpp>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"

namespace Beam {
namespace Reactors {

  /*! \class NoneReactor
      \brief A Reactor that never produces an evaluation.
   */
  template<typename T>
  class NoneReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a NoneReactor.
      NoneReactor() = default;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;
  };

  //! Makes a NoneReactor.
  template<typename T>
  auto MakeNoneReactor() {
    return std::make_shared<NoneReactor<T>>();
  };

  //! Makes a NoneReactor.
  template<typename T>
  auto None() {
    return MakeNoneReactor<int>();
  }

  template<typename T>
  BaseReactor::Update NoneReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == 0) {
      return BaseReactor::Update::COMPLETE;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename T>
  typename NoneReactor<T>::Type NoneReactor<T>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }
}
}

#endif
