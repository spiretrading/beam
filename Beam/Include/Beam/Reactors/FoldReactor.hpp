#ifndef BEAM_FOLD_REACTOR_HPP
#define BEAM_FOLD_REACTOR_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class FoldParameterReactor
      \brief Placeholder Reactors used as parameters to the foldable Reactor.
      \tparam T The type of placeholder to evaluate to.
   */
  template<typename T>
  class FoldParameterReactor : public Reactor<T> {
    public:
      using Type = typename Reactor<T>::Type;

      //! Constructs a FoldParameterReactor.
      FoldParameterReactor();

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      template<typename, typename, typename, typename> friend class FoldReactor;
      Expect<Type> m_value;
      int m_sequenceNumber;
      Expect<Type> m_nextValue;
      int m_nextSequenceNumber;

      void Set(Type value, int sequenceNumber);
      void Set(std::exception_ptr e, int sequenceNumber);
  };

  /*! \class FoldReactor
      \brief Folds the values produced by a Reactor.
      \tparam ProducerReactorType The type of Reactor to fold.
      \tparam EvaluationReactorType The type of Reactor to evaluate.
      \tparam LeftChildReactorType The type of FoldParameterReactor used for the
              left hand side of the evaluation.
      \tparam RightChildReactorType The type of FoldParameterReactor used for
              the right hand side of the evaluation.
   */
  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
  class FoldReactor : public Reactor<GetReactorType<EvaluationReactorType>> {
    public:
      using Type = GetReactorType<EvaluationReactorType>;

      //! Constructs a FoldReactor.
      /*!
        \param producer The Reactor producing the values to fold.
        \param evaluation The Reactor to evaluate.
        \param leftChild The FoldParameterReactor used for the left hand side of
               the evaluation.
        \param rightChild The FoldParameterReactor used for the right hand side
               of the evaluation.
      */
      template<typename ProducerReactorForward,
        typename EvaluationReactorForward, typename LeftChildReactorForward,
        typename RightChildReactorForward>
      FoldReactor(ProducerReactorForward&& producer,
        EvaluationReactorForward&& evaluation,
        LeftChildReactorForward&& leftChild,
        RightChildReactorForward&& rightChild);

      virtual bool IsComplete() const override final;

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      GetOptionalLocalPtr<ProducerReactorType> m_producer;
      GetOptionalLocalPtr<EvaluationReactorType> m_evaluation;
      GetOptionalLocalPtr<LeftChildReactorType> m_leftChild;
      GetOptionalLocalPtr<RightChildReactorType> m_rightChild;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      Expect<Type> m_value;
      boost::optional<Expect<Type>> m_previousValue;
      bool m_hasValue;
      BaseReactor::Update m_state;
  };

  //! Makes a FoldParameterRector.
  template<typename T>
  auto MakeFoldParameterReactor() {
    return std::make_shared<FoldParameterReactor<T>>();
  }

  //! Makes a FoldReactor.
  /*!
    \param producer The Reactor producing the values to fold.
    \param evaluation The Reactor to evaluate.
    \param leftChild The FoldParameterReactor used for the left hand side of the
           evaluation.
    \param rightChild The FoldParameterReactor used for the right hand side of
           the evaluation.
  */
  template<typename ProducerReactor, typename EvaluationReactor,
      typename LeftChildReactor, typename RightChildReactor>
  auto MakeFoldReactor(ProducerReactor&& producer,
      EvaluationReactor&& evaluation, LeftChildReactor&& leftChild,
      RightChildReactor&& rightChild) {
    return std::make_shared<FoldReactor<
      typename std::decay<ProducerReactor>::type,
      typename std::decay<EvaluationReactor>::type,
      typename std::decay<LeftChildReactor>::type,
      typename std::decay<RightChildReactor>::type>>(
      std::forward<ProducerReactor>(producer),
      std::forward<EvaluationReactor>(evaluation),
      std::forward<LeftChildReactor>(leftChild),
      std::forward<RightChildReactor>(rightChild));
  }

  template<typename T>
  FoldParameterReactor<T>::FoldParameterReactor()
      : m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_sequenceNumber{-1},
        m_nextValue{std::make_exception_ptr(ReactorUnavailableException{})},
        m_nextSequenceNumber{-1} {}

  template<typename T>
  bool FoldParameterReactor<T>::IsComplete() const {
    return false;
  }

  template<typename T>
  BaseReactor::Update FoldParameterReactor<T>::Commit(int sequenceNumber) {
    if(sequenceNumber == m_sequenceNumber) {
      return BaseReactor::Update::EVAL;
    } else if(sequenceNumber == m_nextSequenceNumber) {
      m_value = std::move(m_nextValue);
      m_sequenceNumber = sequenceNumber;
      m_nextSequenceNumber = -1;
      return BaseReactor::Update::EVAL;
    }
    return BaseReactor::Update::NONE;
  }

  template<typename T>
  typename FoldParameterReactor<T>::Type FoldParameterReactor<T>::Eval() const {
    return m_value.Get();
  }

  template<typename T>
  void FoldParameterReactor<T>::Set(Type value, int sequenceNumber) {
    m_nextValue = std::move(value);
    m_nextSequenceNumber = sequenceNumber;
  }

  template<typename T>
  void FoldParameterReactor<T>::Set(std::exception_ptr e, int sequenceNumber) {
    m_nextValue = std::move(e);
    m_nextSequenceNumber = sequenceNumber;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
  template<typename ProducerReactorForward, typename EvaluationReactorForward,
    typename LeftChildReactorForward, typename RightChildReactorForward>
  FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftChildReactorType, RightChildReactorType>::FoldReactor(
      ProducerReactorForward&& producer, EvaluationReactorForward&& evaluation,
      LeftChildReactorForward&& leftChild,
      RightChildReactorForward&& rightChild)
      : m_producer{std::forward<ProducerReactorForward>(producer)},
        m_evaluation{std::forward<EvaluationReactorForward>(evaluation)},
        m_leftChild{std::forward<LeftChildReactorForward>(leftChild)},
        m_rightChild{std::forward<RightChildReactorForward>(rightChild)},
        m_currentSequenceNumber{-1},
        m_value{std::make_exception_ptr(ReactorUnavailableException{})},
        m_hasValue{false},
        m_state{BaseReactor::Update::NONE} {}

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
  bool FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftChildReactorType, RightChildReactorType>::IsComplete() const {
    return m_state == BaseReactor::Update::COMPLETE;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
  BaseReactor::Update FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftChildReactorType, RightChildReactorType>::Commit(int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      if(m_hasValue) {
        return BaseReactor::Update::EVAL;
      }
      return BaseReactor::Update::COMPLETE;
    }
    if(IsComplete()) {
      return BaseReactor::Update::NONE;
    }
    auto producerCommit = m_producer->Commit(sequenceNumber);
    if(producerCommit == BaseReactor::Update::NONE) {
      return BaseReactor::Update::NONE;
    } else if(producerCommit == BaseReactor::Update::COMPLETE) {
      m_state = BaseReactor::Update::COMPLETE;
      m_update = BaseReactor::Update::COMPLETE;
      m_currentSequenceNumber = sequenceNumber;
      return BaseReactor::Update::COMPLETE;
    }
    if(!m_previousValue.is_initialized()) {
      m_previousValue = Try(
        [&] {
          return m_producer->Eval();
        });
      m_currentSequenceNumber = sequenceNumber;
      m_update = BaseReactor::Update::NONE;
      return m_update;
    }
    if(m_previousValue->IsException()) {
      m_leftChild->Set(m_previousValue->GetException(), sequenceNumber);
    } else {
      m_leftChild->Set(m_previousValue->Get(), sequenceNumber);
    }
    auto currentValue = Try(
      [&] {
        return m_producer->Eval();
      });
    if(currentValue.IsException()) {
      m_rightChild->Set(currentValue.GetException(), sequenceNumber);
    } else {
      m_rightChild->Set(currentValue.Get(), sequenceNumber);
    }
    m_update = m_evaluation->Commit(sequenceNumber);
    if(m_update == BaseReactor::Update::EVAL) {
      m_value = Try(
        [&] {
          return m_evaluation->Eval();
        });
      m_previousValue = m_value;
    } else if(m_update == BaseReactor::Update::COMPLETE) {
      m_state = BaseReactor::Update::COMPLETE;
    }
    m_currentSequenceNumber = sequenceNumber;
    return m_update;
  }

  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
  typename FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftChildReactorType, RightChildReactorType>::Type
      FoldReactor<ProducerReactorType, EvaluationReactorType,
      LeftChildReactorType, RightChildReactorType>::Eval() const {
    return m_value.Get();
  }
}
}

#endif
