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
    public:
      using Source = typename QueueWriter<T>::Source;

      //! Constructs a FromPythonQueueWriter.
      /*!
        \param target The QueueWriter to wrap.
      */
      FromPythonQueueWriter(
        const std::shared_ptr<QueueWriter<boost::python::object>>& target);

      virtual ~FromPythonQueueWriter() override final;

      //! Returns the QueueWriter being wrapped.
      const std::shared_ptr<QueueWriter<boost::python::object>>&
        GetTarget() const;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    private:
      std::shared_ptr<QueueWriter<boost::python::object>> m_target;
  };

  template<typename T>
  FromPythonQueueWriter<T>::FromPythonQueueWriter(
      const std::shared_ptr<QueueWriter<boost::python::object>>& target)
      : m_target{target} {}

  template<typename T>
  FromPythonQueueWriter<T>::~FromPythonQueueWriter() {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    m_target.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueWriter<boost::python::object>>&
      FromPythonQueueWriter<T>::GetTarget() const {
    return m_target;
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(const Source& value) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    m_target->Push(boost::python::object{value});
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(Source&& value) {
    GilLock gil;
    boost::lock_guard<GilLock> lock{gil};
    m_target->Push(boost::python::object{std::move(value)});
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    m_target->Break(e);
  }
}

#endif
