#ifndef BEAM_CALLBACK_WRITER_QUEUE_HPP
#define BEAM_CALLBACK_WRITER_QUEUE_HPP
#include <functional>
#include <boost/optional/optional.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Queues/Queues.hpp"
#include "Beam/Queues/QueueWriter.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam {

  /*! \class CallbackWriterQueue
      \brief Used to invoke a callback when data is pushed onto this Queue.
      \tparam T The type of data being pushed onto the Queue.
   */
  template<typename T>
  class CallbackWriterQueue : public QueueWriter<T> {
    public:

      //! The type of data being pushed onto the Queue.
      using Source = T;

      //! The function to call when data is pushed onto this Queue.
      /*!
        \param value The value that was pushed onto the Queue.
      */
      using CallbackFunction = std::function<void (const Source& source)>;

      //! The function to call when this Queue is broken.
      /*!
        \param e Stores the reason for the break.
      */
      using BreakFunction = std::function<void (const std::exception_ptr& e)>;

      //! Constructs a CallbackWriterQueue.
      /*!
        \param callback The function to call when data is pushed onto this
               Queue.
      */
      CallbackWriterQueue(const CallbackFunction& callback);

      //! Constructs a CallbackWriterQueue.
      /*!
        \param callback The function to call when data is pushed onto this
               Queue.
        \param breakCallback The function to call when this Queue is broken.
      */
      CallbackWriterQueue(const CallbackFunction& callback,
        const BreakFunction& breakCallback);

      virtual ~CallbackWriterQueue() override final;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& exception) override final;

      using QueueWriter<T>::Break;
    private:
      struct Callbacks {
        CallbackFunction m_callback;
        BreakFunction m_breakCallback;
      };
      mutable Threading::RecursiveMutex m_mutex;
      bool m_isBroken;
      boost::optional<Callbacks> m_callbacks;
  };

  template<typename T>
  CallbackWriterQueue<T>::CallbackWriterQueue(const CallbackFunction& callback)
      : CallbackWriterQueue(callback, [] (const std::exception_ptr&) {}) {}

  template<typename T>
  CallbackWriterQueue<T>::CallbackWriterQueue(const CallbackFunction& callback,
      const BreakFunction& breakCallback)
      : m_isBroken{false},
        m_callbacks{{callback, breakCallback}} {}

  template<typename T>
  CallbackWriterQueue<T>::~CallbackWriterQueue() {
    Break();
  }

  template<typename T>
  void CallbackWriterQueue<T>::Push(const Source& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(value);
  }

  template<typename T>
  void CallbackWriterQueue<T>::Push(Source&& value) {
    boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
    if(m_isBroken) {
      return;
    }
    m_callbacks->m_callback(std::move(value));
  }

  template<typename T>
  void CallbackWriterQueue<T>::Break(const std::exception_ptr& exception) {
    {
      boost::lock_guard<Threading::RecursiveMutex> lock{m_mutex};
      if(m_isBroken) {
        return;
      }
      m_isBroken = true;
    }
    m_callbacks->m_breakCallback(exception);
    m_callbacks.reset();
  }
}

#endif
