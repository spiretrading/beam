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

        Child(BaseReactor& reactor);
      };
      std::vector<Child> m_children;
      BaseReactor::Update m_value;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
  };

  inline CommitReactor::Child::Child(BaseReactor& reactor)
      : m_reactor{&reactor},
        m_isInitialized{false} {}

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
    } else if(m_state & BaseReactor::Update::COMPLETE) {
      return BaseReactor::Update::NONE;
    }
    auto update =
      [&] {
        if(m_state == BaseReactor::Update::NONE) {
          auto update = BaseReactor::Update::EVAL;
          for(auto& child : m_children) {
            auto childUpdate = child.m_reactor->Commit(sequenceNumber);
            if(!child.m_isInitialized) {
              if(childUpdate == BaseReactor::Update::EVAL) {
                child.m_isInitialized = true;
              } else if(childUpdate == BaseReactor::Update::COMPLETE) {
                update = BaseReactor::Update::COMPLETE;
              } else if(update != BaseReactor::Update::COMPLETE) {
                update = BaseReactor::Update::NONE;
              }
            }
          }
          if(update == BaseReactor::Update::EVAL) {
            m_state = BaseReactor::Update::EVAL;
          }
          return update;
        }
        auto update = BaseReactor::Update::NONE;
        auto hasCompletion = false;
        for(auto& child : m_children) {
          auto childUpdate = child.m_reactor->Commit(sequenceNumber);
          if(childUpdate == BaseReactor::Update::EVAL) {
            update = BaseReactor::Update::EVAL;
          } else if(childUpdate == BaseReactor::Update::COMPLETE) {
            hasCompletion = true;
          }
        }
        if(update == BaseReactor::Update::NONE && hasCompletion) {
          if(AreParametersComplete()) {
            update = BaseReactor::Update::COMPLETE;
          }
        }
        return update;
      }();
    m_update = update;
    if(m_update == BaseReactor::Update::EVAL) {
      if(AreParametersComplete()) {
        m_state = BaseReactor::Update::COMPLETE;
      }
      m_hasValue = true;
    } else if(m_update == BaseReactor::Update::COMPLETE) {
      m_state = BaseReactor::Update::COMPLETE;
    }
    m_currentSequenceNumber = sequenceNumber;
    return m_update;
  }

  inline CommitReactor::Type CommitReactor::Eval() const {
    return m_update;
  }
}
}

#endif
