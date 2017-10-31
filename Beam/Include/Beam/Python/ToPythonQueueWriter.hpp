#ifndef BEAM_TO_PYTHON_QUEUE_WRITER_HPP
#define BEAM_TO_PYTHON_QUEUE_WRITER_HPP
#include <type_traits>
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {
namespace Details {
  template<typename T, typename Enabled = void>
  struct Extractor;

  template<typename T>
  struct Extractor<T, typename std::enable_if<
      std::is_constructible<T, boost::python::object>::value>::type> {
    T operator ()(const boost::python::object& value) {
      return T{value};
    }
  };

  template<typename T, typename Enabled>
  struct Extractor {
    T operator ()(const boost::python::object& value) {
      return boost::python::extract<T>(value)();
    }
  };
}

  /*! \class ToPythonQueueWriter
      \brief Wraps a QueueWriter of type T to a QueueWriter of Python objects.
      \tparam T The type of data to push onto the queue.
   */
  template<typename T>
  class ToPythonQueueWriter : public QueueWriter<boost::python::object> {
    public:
      using Source = typename QueueWriter<boost::python::object>::Source;

      using Type = T;

      //! Constructs a ToPythonQueueWriter.
      /*!
        \param target The QueueWriter to wrap.
      */
      ToPythonQueueWriter(std::shared_ptr<QueueWriter<Type>> target);

      virtual ~ToPythonQueueWriter() override final = default;

      //! Returns the QueueWriter being wrapped.
      const std::shared_ptr<QueueWriter<Type>>& GetTarget() const;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    private:
      std::shared_ptr<QueueWriter<Type>> m_target;
  };

  template<typename Type>
  auto MakeToPythonQueueWriter(std::shared_ptr<QueueWriter<Type>> target) {
    return std::make_shared<ToPythonQueueWriter<Type>>(std::move(target));
  }

  template<typename T>
  ToPythonQueueWriter<T>::ToPythonQueueWriter(
      std::shared_ptr<QueueWriter<Type>> target)
      : m_target{std::move(target)} {}

  template<typename T>
  const std::shared_ptr<QueueWriter<typename ToPythonQueueWriter<T>::Type>>&
      ToPythonQueueWriter<T>::GetTarget() const {
    return m_target;
  }

  template<typename T>
  void ToPythonQueueWriter<T>::Push(const Source& value) {
    m_target->Push(Details::Extractor<Type>{}(value));
  }

  template<typename T>
  void ToPythonQueueWriter<T>::Push(Source&& value) {
    m_target->Push(Details::Extractor<Type>{}(value));
  }

  template<typename T>
  void ToPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    m_target->Break(e);
  }
}

#endif
