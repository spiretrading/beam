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

      virtual bool IsInitialized() const override;

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;
  };

  //! Makes a NoneReactor.
  template<typename T>
  auto MakeNoneReactor() {
    return std::make_shared<NoneReactor<T>>();
  };

  template<typename T>
  bool NoneReactor<T>::IsInitialized() const {
    return true;
  }

  template<typename T>
  bool NoneReactor<T>::IsComplete() const {
    return true;
  }

  template<typename T>
  BaseReactor::Update NoneReactor<T>::Commit(int sequenceNumber) {
    return BaseReactor::Update::NONE;
  }

  template<typename T>
  typename NoneReactor<T>::Type NoneReactor<T>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }
}
}

#endif
