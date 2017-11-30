#ifndef BEAM_COMMIT_REACTOR_HPP
#define BEAM_COMMIT_REACTOR_HPP
#include <vector>
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class CommitReactor
      \brief Helper class used to determine the aggregate state of a list of
             Reactors.
   */
  class CommitReactor : public Reactor<BaseReactor::Update> {
    public:

      //! Constructs a CommitReactor.
      /*!
        \param children The Reactors to aggregate.
      */
      CommitReactor(const std::vector<BaseReactor*>& children);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      struct Child {
        BaseReactor* m_reactor;
        bool m_isInitialized;
        bool m_isComplete;

        Child(BaseReactor& reactor);
      };
      std::vector<Child> m_children;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
  };

  inline CommitReactor::Child::Child(BaseReactor& reactor)
      : m_reactor{&reactor},
        m_isInitialized{false},
        m_isComplete{false} {}

  inline CommitReactor::CommitReactor(const std::vector<BaseReactor*>& children)
      : m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {
    for(auto& child : children) {
      m_children.emplace_back(*child);
    }
  }

  inline BaseReactor::Update CommitReactor::Commit(int sequenceNumber) {
    if(m_currentSequenceNumber == sequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      return m_state;
    } else if(IsComplete(m_state)) {
      return BaseReactor::Update::NONE;
    }
    m_update =
      [&] {
        auto update = [&] {
          if(m_state == BaseReactor::Update::NONE) {
            return BaseReactor::Update::EVAL;
          } else {
            return BaseReactor::Update::NONE;
          }
        }();
        auto completeCount = 0;
        for(auto& child : m_children) {
          if(child.m_isComplete) {
            ++completeCount;
            continue;
          }
          auto childUpdate = child.m_reactor->Commit(sequenceNumber);
          if(IsComplete(childUpdate)) {
            ++completeCount;
            child.m_isComplete = true;
          }
          if(!child.m_isInitialized) {
            if(HasEval(childUpdate)) {
              child.m_isInitialized = true;
            } else if(child.m_isComplete) {
              update = BaseReactor::Update::COMPLETE;
            } else {
              update = BaseReactor::Update::NONE;
            }
          } else if(m_state != BaseReactor::Update::NONE) {
            if(HasEval(childUpdate)) {
              update = BaseReactor::Update::EVAL;
            }
          }
        }
        if(completeCount == static_cast<int>(m_children.size())) {
          Combine(update, BaseReactor::Update::COMPLETE);
        }
        return update;
      }();
    m_currentSequenceNumber = sequenceNumber;
    Combine(m_state, m_update);
    return m_update;
  }

  inline CommitReactor::Type CommitReactor::Eval() const {
    return m_update;
  }
}
}

#endif
