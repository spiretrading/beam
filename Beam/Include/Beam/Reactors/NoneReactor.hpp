#ifndef BEAM_NONEREACTOR_HPP
#define BEAM_NONEREACTOR_HPP
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
      typedef GetReactorType<Reactor<T>> Type;

      //! Constructs a NoneReactor.
      NoneReactor();

      virtual Type Eval() const;
  };

  //! Makes a NoneReactor.
  template<typename T>
  std::shared_ptr<NoneReactor<T>> MakeNoneReactor() {
    return std::make_shared<NoneReactor<T>>();
  };

  template<typename T>
  NoneReactor<T>::NoneReactor() {
    this->SetComplete();
  }

  template<typename T>
  typename NoneReactor<T>::Type NoneReactor<T>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException());
  }
}
}

#endif
