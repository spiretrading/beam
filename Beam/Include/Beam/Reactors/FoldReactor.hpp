#ifndef BEAM_FOLDREACTOR_HPP
#define BEAM_FOLDREACTOR_HPP
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
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
        typename EvaluationReactorForward,
        typename LeftTriggerReactorForward, typename RightTriggerReactorForward>
      FoldReactor(ProducerReactorForward&& producer,
        EvaluationReactorForward&& evaluation,
        LeftTriggerReactorForward&& leftTrigger,
        RightTriggerReactorForward&& rightTrigger);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      using LeftType = GetReactorType<LeftTriggerReactorType>;
      using RightType = GetReactorType<RightTriggerReactorType>;
      ReactorContainer<ProducerReactorType> m_producer;
      ReactorContainer<EvaluationReactorType> m_evaluation;
      GetOptionalLocalPtr<LeftTriggerReactorType> m_leftTrigger;
      GetOptionalLocalPtr<RightTriggerReactorType> m_rightTrigger;
      bool m_commit;
      Expect<LeftType> m_previousValue;
      bool m_hasPreviousValue;
      Expect<Type> m_value;

      void OnUpdate();
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
  std::shared_ptr<FoldReactor<typename std::decay<ProducerReactor>::type,
      typename std::decay<EvaluationReactor>::type,
      typename std::decay<LeftTriggerReactor>::type,
      typename std::decay<RightTriggerReactor>::type>> MakeFoldReactor(
      ProducerReactor&& producer, EvaluationReactor&& evaluation,
      LeftTriggerReactor&& leftTrigger, RightTriggerReactor&& rightTrigger) {
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
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_producer(std::forward<ProducerReactorForward>(producer), *this),
        m_evaluation(std::forward<EvaluationReactorForward>(evaluation),
          std::bind(&FoldReactor::OnUpdate, this)),
        m_leftTrigger(std::forward<LeftTriggerReactorForward>(leftTrigger)),
        m_rightTrigger(
          std::forward<RightTriggerReactorForward>(rightTrigger)),
        m_commit(false),
        m_hasPreviousValue(false) {}
BEAM_UNSUPPRESS_THIS_INITIALIZER()

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  void FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::Commit() {
    m_commit = true;
    auto producerUpdated = m_producer.Commit();
    if(!producerUpdated) {
      m_evaluation.Commit();
      m_commit = false;
      return;
    }
    if(!m_hasPreviousValue) {
      m_previousValue = m_producer.GetValue();
      m_hasPreviousValue = true;
      m_evaluation.Commit();
      m_commit = false;
      return;
    }
    if(m_previousValue.IsException()) {
      m_leftTrigger->SetException(m_previousValue.GetException());
    } else {
      m_leftTrigger->SetValue(m_previousValue.Get());
    }
    m_leftTrigger->Execute();
    auto& currentValue = m_producer.GetValue();
    if(currentValue.IsException()) {
      m_rightTrigger->SetException(currentValue.GetException());
    } else {
      m_rightTrigger->SetValue(currentValue.Get());
    }
    m_rightTrigger->Execute();
    auto evaluationUpdated = m_evaluation.Commit();
    if(evaluationUpdated) {
      this->IncrementSequenceNumber();
      m_value = m_evaluation.GetValue();
      if(!m_value.IsException()) {
        m_previousValue = m_value;
      }
    }
    if(m_producer.IsComplete()) {
      this->SetComplete();
    }
    m_commit = false;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  typename FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::Type FoldReactor<
      ProducerReactorType, EvaluationReactorType, LeftTriggerReactorType,
      RightTriggerReactorType>::Eval() const {
    return m_value.Get();
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
  void FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftTriggerReactorType, RightTriggerReactorType>::OnUpdate() {
    if(m_commit) {
      return;
    }
    this->SignalUpdate();
  }
}
}

#endif
