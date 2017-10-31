#ifndef BEAM_TO_PYTHON_QUEUE_READER_HPP
#define BEAM_TO_PYTHON_QUEUE_READER_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam {

  /*! \class ToPythonQueueReader
      \brief Wraps a QueueReader of type T to a QueueReader of Python objects.
   */
  template<typename T>
  class ToPythonQueueReader : public QueueReader<boost::python::object> {
    public:

      //! The type being read.
      using Target = typename QueueReader<boost::python::object>::Target;

      //! The type of the QueueReader being wrapped.
      using Type = T;

      //! Constructs a ToPythonQueueReader.
      /*!
        \param source The QueueReader to wrap.
      */
      ToPythonQueueReader(std::shared_ptr<QueueReader<Type>> source);

      virtual ~ToPythonQueueReader() override final = default;

      //! Returns the QueueReader being wrapped.
      const std::shared_ptr<QueueReader<Type>>& GetSource() const;

      virtual bool IsEmpty() const override final;

      virtual Target Top() const override final;

      virtual void Pop() override final;

      virtual void Break(const std::exception_ptr& e) override final;

    protected:
      virtual bool IsAvailable() const override final;

    private:
      std::shared_ptr<QueueReader<Type>> m_source;
  };

  //! Makes a ToPythonQueueReader.
  /*!
    \param source The QueueReader to wrap.
  */
  template<typename T>
  auto MakeToPythonQueueReader(std::shared_ptr<QueueReader<T>> source) {
    return std::make_shared<ToPythonQueueReader<T>>(std::move(source));
  }

  template<typename T>
  ToPythonQueueReader<T>::ToPythonQueueReader(
      std::shared_ptr<QueueReader<Type>> source)
      : m_source{std::move(source)} {}

  template<typename T>
  const std::shared_ptr<QueueReader<typename ToPythonQueueReader<T>::Type>>&
      ToPythonQueueReader<T>::GetSource() const {
    return m_source;
  }

  template<typename T>
  bool ToPythonQueueReader<T>::IsEmpty() const {
    return m_source->IsEmpty();
  }

  template<typename T>
  typename ToPythonQueueReader<T>::Target ToPythonQueueReader<T>::Top() const {
    return boost::python::object{[&] {
      Python::GilRelease gil;
      boost::lock_guard<Python::GilRelease> release{gil};
      return m_source->Top();
    }()};
  }

  template<typename T>
  void ToPythonQueueReader<T>::Pop() {
    m_source->Pop();
  }

  template<typename T>
  void ToPythonQueueReader<T>::Break(const std::exception_ptr& e) {
    m_source->Break(e);
  }

  template<typename T>
  bool ToPythonQueueReader<T>::IsAvailable() const {
    return QueueReader<boost::python::object>::IsAvailable(*m_source);
  }
}

#endif
