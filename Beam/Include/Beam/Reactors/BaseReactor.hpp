#ifndef BEAM_BASEREACTOR_HPP
#define BEAM_BASEREACTOR_HPP
#include <limits>
#include <typeinfo>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {

  /*! \class BaseReactor
      \brief The untyped base class of a Reactor.
   */
  class BaseReactor : private boost::noncopyable {
    public:

      //! Signals that this Reactor has been updated.
      using UpdateSignal = boost::signals2::signal<void ()>;

      virtual ~BaseReactor() = default;

      //! Returns the current sequence number.
      unsigned int GetSequenceNumber() const;

      //! Returns <code>true</code> iff this Reactor is initializing.
      bool IsInitializing() const;

      //! Returns <code>true</code> iff this Reactor is initialized.
      bool IsInitialized() const;

      //! Returns <code>true</code> iff this Reactor can be evaluated.
      bool HasEvaluation() const;

      //! Returns <code>true</code> iff this Reactor is complete.
      bool IsComplete() const;

      //! Returns the evaluation's type_info.
      virtual const std::type_info& GetType() const = 0;

      //! Commits changes to this Reactor.
      virtual void Commit() = 0;

      //! Returns the generic value.
      virtual Expect<void> GetBaseValue() = 0;

      //! Connects a slot to the UpdateSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_type& slot) const;

    protected:

      //! Constructs a BaseReactor.
      BaseReactor();

      //! Updates the sequence number.
      void IncrementSequenceNumber();

      //! Indicates no more updates to this Reactor will be made.
      void SetComplete();

      //! Signals that an update occurred.
      void SignalUpdate() const;

    private:
      template<typename> friend class ReactorContainer;
      unsigned int m_sequenceNumber;
      bool m_isComplete;
      mutable UpdateSignal m_updateSignal;
  };

  inline unsigned int BaseReactor::GetSequenceNumber() const {
    return m_sequenceNumber;
  }

  inline bool BaseReactor::IsInitializing() const {
    return !IsInitialized();
  }

  inline bool BaseReactor::IsInitialized() const {
    return HasEvaluation() || IsComplete();
  }

  inline bool BaseReactor::HasEvaluation() const {
    return m_sequenceNumber != 0;
  }

  inline bool BaseReactor::IsComplete() const {
    return m_isComplete;
  }

  inline boost::signals2::connection BaseReactor::ConnectUpdateSignal(
      const UpdateSignal::slot_type& slot) const {
    return m_updateSignal.connect(slot);
  }

  inline BaseReactor::BaseReactor()
      : m_sequenceNumber{0},
        m_isComplete{false} {}

  inline void BaseReactor::IncrementSequenceNumber() {
    ++m_sequenceNumber;
  }

  inline void BaseReactor::SetComplete() {
    m_isComplete = true;
  }

  inline void BaseReactor::SignalUpdate() const {
    m_updateSignal();
  }
}
}

#endif
