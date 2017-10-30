#ifndef BEAM_FROM_PYTHON_QUEUE_WRITER_HPP
#define BEAM_FROM_PYTHON_QUEUE_WRITER_HPP
#include "Beam/Python/GilLock.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {

  /*! \class FromPythonQueueWriter
      \brief Wraps a QueueWriter of Python objects to a QueueWriter of type T.
      \tparam T The type of data to push onto the queue.
   */
  template<typename T>
  class FromPythonQueueWriter : public QueueWriter<T> {
    private:
      struct Guard {};

    public:
      using Source = typename QueueWriter<T>::Source;

      //! Constructs a FromPythonQueueWriter.
      /*!
        \param target The QueueWriter to wrap.
      */
      FromPythonQueueWriter(
        std::shared_ptr<QueueWriter<boost::python::object>> target, Guard);

      virtual ~FromPythonQueueWriter() override final;

      //! Returns the QueueWriter being wrapped.
      std::shared_ptr<QueueWriter<boost::python::object>> GetTarget() const;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    private:
      template<typename U> friend std::shared_ptr<FromPythonQueueWriter<U>>
        MakeFromPythonQueueWriter(
        std::shared_ptr<QueueWriter<boost::python::object>> target);
      std::shared_ptr<void> m_self;
      std::weak_ptr<QueueWriter<boost::python::object>> m_target;

      void Bind(std::shared_ptr<void> self);
  };

  //! Constructs a FromPythonQueueWriter.
  /*!
    \param target The QueueWriter to wrap.
  */
  template<typename T>
  std::shared_ptr<FromPythonQueueWriter<T>> MakeFromPythonQueueWriter(
      std::shared_ptr<QueueWriter<boost::python::object>> target) {
    auto queue = std::make_shared<FromPythonQueueWriter<T>>(std::move(target),
      typename FromPythonQueueWriter<T>::Guard{});
    queue->Bind(queue);
    return queue;
  }

  template<typename T>
  FromPythonQueueWriter<T>::FromPythonQueueWriter(
      std::shared_ptr<QueueWriter<boost::python::object>> target, Guard)
      : m_target{std::move(target)} {}

  template<typename T>
  FromPythonQueueWriter<T>::~FromPythonQueueWriter() {
    auto target = m_target.lock();
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    target.reset();
  }

  template<typename T>
  std::shared_ptr<QueueWriter<boost::python::object>>
      FromPythonQueueWriter<T>::GetTarget() const {
    return m_target.lock();
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(const Source& value) {
    auto target = m_target.lock();
    if(target == nullptr) {
      m_self.reset();
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    target->Push(boost::python::object{value});
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(Source&& value) {
    auto target = m_target.lock();
    if(target == nullptr) {
      m_self.reset();
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    target->Push(boost::python::object{std::move(value)});
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    auto target = m_target.lock();
    if(target == nullptr) {
      m_self.reset();
      return;
    }
    target->Break(e);
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
  }
}

#endif
