#ifndef BEAM_UPDATE_REACTOR_HPP
#define BEAM_UPDATE_REACTOR_HPP
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Reactors {

  /*! \class UpdateReactor
      \brief A Reactor that evaluates to another Reactor's most recent update.
   */
  class UpdateReactor : public Reactor<BaseReactor::Update> {
    public:

      //! Constructs an UpdateReactor.
      /*!
        \param reactor The Reactor to monitor for updates.
      */
      UpdateReactor(std::shared_ptr<BaseReactor> reactor);

      virtual BaseReactor::Update Commit(int sequenceNumber) override final;

      virtual Type Eval() const override final;

    private:
      std::shared_ptr<BaseReactor> m_reactor;
      BaseReactor::Update m_value;
      int m_currentSequenceNumber;
      BaseReactor::Update m_update;
      BaseReactor::Update m_state;
  };

  //! Makes an UpdateReactor.
  /*!
    \param reactor The Reactor to monitor for updates.
  */
  inline auto MakeUpdateReactor(std::shared_ptr<BaseReactor> reactor) {
    return std::make_shared<UpdateReactor>(std::move(reactor));
  }

  inline UpdateReactor::UpdateReactor(std::shared_ptr<BaseReactor> reactor)
      : m_reactor{std::move(reactor)},
        m_value{BaseReactor::Update::NONE},
        m_currentSequenceNumber{-1},
        m_state{BaseReactor::Update::NONE} {}

  inline BaseReactor::Update UpdateReactor::Commit(int sequenceNumber) {
    if(m_currentSequenceNumber == sequenceNumber) {
      return m_update;
    } else if(sequenceNumber == 0 && m_currentSequenceNumber != -1) {
      return m_state;
    } else if(IsComplete(m_state)) {
      return BaseReactor::Update::NONE;
    }
    auto update = m_reactor->Commit(sequenceNumber);
    if(m_state != BaseReactor::Update::NONE) {
      if(update == BaseReactor::Update::NONE) {
        if(m_value == BaseReactor::Update::NONE) {
          return BaseReactor::Update::NONE;
        }
        m_value = BaseReactor::Update::NONE;
        update = BaseReactor::Update::EVAL;
      } else {
        m_value = update;
        Combine(update, BaseReactor::Update::EVAL);
      }
    } else if(update == BaseReactor::Update::NONE) {
      return BaseReactor::Update::NONE;
    } else {
      m_value = update;
      Combine(update, BaseReactor::Update::EVAL);
    }
    m_update = update;
    m_currentSequenceNumber = sequenceNumber;
    Combine(m_state, update);
    return update;
  }

  inline UpdateReactor::Type UpdateReactor::Eval() const {
    return m_value;
  }
}
}

#endif
