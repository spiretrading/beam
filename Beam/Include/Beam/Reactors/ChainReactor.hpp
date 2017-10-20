#ifndef BEAM_CHAIN_REACTOR_HPP
#define BEAM_CHAIN_REACTOR_HPP
#include <type_traits>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ChainReactor
      \brief A Reactor that produces values from one Reactor until it completes
             and then produces values from another Reactor.
      \tparam InitialReactorType The Reactor to initially evaluate to.
      \tparam ContinuationReactorType The Reactor to evaluate to thereafter.
   */
  template<typename InitialReactorType, typename ContinuationReactorType>
  class ChainReactor : public Reactor<GetReactorType<InitialReactorType>> {
    public:
      using Type = GetReactorType<InitialReactorType>;

      //! Constructs a ChainReactor.
      /*!
        \param initialReactor The Reactor to initially evaluate to.
        \param continuationReactor The Reactor to evaluate to thereafter.
      */
      template<typename InitialReactorForward,
        typename ContinuationReactorForward>
      ChainReactor(InitialReactorForward&& initialReactor,
        ContinuationReactorForward&& continuationReactor);

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

    private:
      GetOptionalLocalPtr<InitialReactorType> m_initialReactor;
      GetOptionalLocalPtr<ContinuationReactorType> m_continuationReactor;
  };

  //! Makes a ChainReactor.
  /*!
    \param initialReactor The Reactor to initially evaluate to.
    \param continuationReactor The Reactor to evaluate to thereafter.
  */
  template<typename InitialReactor, typename ContinuationReactor>
  auto MakeChainReactor(InitialReactor&& initialReactor,
      ContinuationReactor&& continuationReactor) {
    return std::make_shared<ChainReactor<
      typename std::decay<InitialReactor>::type,
      typename std::decay<ContinuationReactor>::type>>(
      std::forward<InitialReactor>(initialReactor),
      std::forward<ContinuationReactor>(continuationReactor));
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  template<typename InitialReactorForward, typename ContinuationReactorForward>
  ChainReactor<InitialReactorType, ContinuationReactorType>::ChainReactor(
      InitialReactorForward&& initialReactor,
      ContinuationReactorForward&& continuationReactor)
      : m_initialReactor{std::forward<InitialReactorForward>(initialReactor)},
        m_continuationReactor{
          std::forward<ContinuationReactorForward>(continuationReactor)} {}

  template<typename InitialReactorType, typename ContinuationReactorType>
  bool ChainReactor<InitialReactorType, ContinuationReactorType>::
      IsComplete() const {
    return false;
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  BaseReactor::Update ChainReactor<InitialReactorType,
      ContinuationReactorType>::Commit(int sequenceNumber) {
    return BaseReactor::Update::NONE;
  }

  template<typename InitialReactorType, typename ContinuationReactorType>
  typename ChainReactor<InitialReactorType, ContinuationReactorType>::Type
      ChainReactor<InitialReactorType, ContinuationReactorType>::Eval() const {
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }
}
}

#endif
