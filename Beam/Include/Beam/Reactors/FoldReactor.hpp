#ifndef BEAM_FOLD_REACTOR_HPP
#define BEAM_FOLD_REACTOR_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class FoldReactor
      \brief Folds the values produced by a Reactor.
      \tparam ProducerReactorType The type of Reactor to fold.
      \tparam EvaluationReactorType The type of Reactor to evaluate.
      \tparam LeftTriggerReactorType The type of TriggerReactor used for the
              left hand side of the evaluation.
      \tparam RightTriggerReactorType The type of TriggerReactor used for the
              right hand side of the evaluation.
   */
  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  class FoldReactor : public Reactor<GetReactorType<EvaluationReactorType>> {
    public:
      using Type = GetReactorType<EvaluationReactorType>;

      //! Constructs a FoldReactor.
      /*!
        \param producer The Reactor producing the values to fold.
        \param evaluation The Reactor to evaluate.
        \param leftTrigger The TriggerReactor used for the left hand side of the
               evaluation.
        \param rightTrigger The TriggerReactor used for the right hand side of
               the evaluation.
      */
      template<typename ProducerReactorForward,
        typename EvaluationReactorForward, typename LeftTriggerReactorForward,
        typename RightTriggerReactorForward>
      FoldReactor(ProducerReactorForward&& producer,
        EvaluationReactorForward&& evaluation,
        LeftTriggerReactorForward&& leftTrigger,
        RightTriggerReactorForward&& rightTrigger);

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      using LeftType = GetReactorType<LeftTriggerReactorType>;
      using RightType = GetReactorType<RightTriggerReactorType>;
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      GetOptionalLocalPtr<EvaluationReactorType> m_evaluation;
      GetOptionalLocalPtr<LeftTriggerReactorType> m_leftTrigger;
      GetOptionalLocalPtr<RightTriggerReactorType> m_rightTrigger;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      Expect<Type> m_value;
      bool m_hasValue;
  };

  //! Makes a FoldReactor.
  /*!
    \param producer The Reactor producing the values to fold.
    \param evaluation The Reactor to evaluate.
    \param leftTrigger The TriggerReactor used for the left hand side of the
           evaluation.
    \param rightTrigger The TriggerReactor used for the right hand side of the
           evaluation.
  */
  template<typename ProducerReactor, typename EvaluationReactor,
      typename LeftTriggerReactor, typename RightTriggerReactor>
  auto MakeFoldReactor(ProducerReactor&& producer,
      EvaluationReactor&& evaluation, LeftTriggerReactor&& leftTrigger,
      RightTriggerReactor&& rightTrigger) {
    return std::make_shared<FoldReactor<
      typename std::decay<ProducerReactor>::type,
      typename std::decay<EvaluationReactor>::type,
      typename std::decay<LeftTriggerReactor>::type,
      typename std::decay<RightTriggerReactor>::type>>(
      std::forward<ProducerReactor>(producer),
      std::forward<EvaluationReactor>(evaluation),
      std::forward<LeftTriggerReactor>(leftTrigger),
      std::forward<RightTriggerReactor>(rightTrigger));
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  template<typename ProducerReactorForward, typename EvaluationReactorForward,
    typename LeftTriggerReactorForward, typename RightTriggerReactorForward>
  FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::FoldReactor(
      ProducerReactorForward&& producer, EvaluationReactorForward&& evaluation,
      LeftTriggerReactorForward&& leftTrigger,
      RightTriggerReactorForward&& rightTrigger)
      : m_producer{std::forward<ProducerReactorForward>(producer)},
        m_evaluation{std::forward<EvaluationReactorForward>(evaluation)},
        m_leftTrigger{std::forward<LeftTriggerReactorForward>(leftTrigger)},
        m_rightTrigger{
          std::forward<RightTriggerReactorForward>(rightTrigger)},
        m_currentSequenceNumber{-1},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false} {}

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  bool FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::IsComplete() const {
    return false;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  BaseReactor::Update FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::Commit(
      int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      if(m_hasValue) {
        return BaseReactor::Update::EVAL;
      }
      return BaseReactor::Update::COMPLETE;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  typename FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::Type
      FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
