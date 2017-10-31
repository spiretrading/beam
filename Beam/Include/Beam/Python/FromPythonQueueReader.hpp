#ifndef BEAM_FROM_PYTHON_QUEUE_READER_HPP
#define BEAM_FROM_PYTHON_QUEUE_READER_HPP
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /*! \class FromPythonQueueReader
      \brief Wraps a QueueReader of Python objects to a QueueReader of type T.
   */
  template<typename T>
  class FromPythonQueueReader : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      //! Constructs a FromPythonQueueReader.
      /*!
        \param source The QueueReader to wrap.
      */
      FromPythonQueueReader(
        std::shared_ptr<QueueReader<boost::python::object>> source);

      virtual ~FromPythonQueueReader() override final;

      //! Returns the QueueReader being wrapped.
      const std::shared_ptr<QueueReader<boost::python::object>>&
        GetSource() const;

      virtual bool IsEmpty() const override final;

      virtual Target Top() const override final;

      virtual void Pop() override final;

      virtual void Break(const std::exception_ptr& e) override final;

    protected:
      virtual bool IsAvailable() const override final;

    private:
      std::shared_ptr<QueueReader<boost::python::object>> m_source;
  };

  //! Makes a FromPythonQueueReader.
  /*!
    \param source The QueueReader to wrap.
  */
  template<typename T>
  auto MakeFromPythonQueueReader(
      std::shared_ptr<QueueReader<boost::python::object>> source) {
    return std::make_shared<FromPythonQueueReader<T>>(std::move(source));
  }

  template<typename T>
  FromPythonQueueReader<T>::FromPythonQueueReader(
      std::shared_ptr<QueueReader<boost::python::object>> source)
      : m_source{std::move(source)} {}

  template<typename T>
  FromPythonQueueReader<T>::~FromPythonQueueReader() {
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    m_source.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueReader<boost::python::object>>&
      FromPythonQueueReader<T>::GetSource() const {
    return m_source;
  }

  template<typename T>
  bool FromPythonQueueReader<T>::IsEmpty() const {
    return m_source->IsEmpty();
  }

  template<typename T>
  typename FromPythonQueueReader<T>::Target
      FromPythonQueueReader<T>::Top() const {
    if(IsEmpty()) {
      Python::GilRelease gil;
      boost::lock_guard<Python::GilRelease> lock{gil};
      m_source->Wait();
    }
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    return boost::python::extract<T>(m_source->Top())();
  }

  template<typename T>
  void FromPythonQueueReader<T>::Pop() {
    m_source->Pop();
  }

  template<typename T>
  void FromPythonQueueReader<T>::Break(const std::exception_ptr& e) {
    m_source->Break(e);
  }

  template<typename T>
  bool FromPythonQueueReader<T>::IsAvailable() const {
    return QueueReader<T>::IsAvailable(*m_source);
  }
}

#endif
