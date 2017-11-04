#ifndef BEAM_FROM_PYTHON_QUEUE_WRITER_HPP
#define BEAM_FROM_PYTHON_QUEUE_WRITER_HPP
#include <type_traits>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam {
namespace Details {
  template<typename T, typename Enabled = void>
  struct ToObject;

  template<typename T>
  struct ToObject<T, typename std::enable_if<std::is_pointer<
      typename std::decay<T>::type>::value>::type> {
    template<typename U>
    boost::python::object operator ()(U&& value) const {
      return boost::python::object{boost::python::ptr(value)};
    }
  };

  template<typename T, typename Enabled>
  struct ToObject {
    template<typename U>
    boost::python::object operator ()(U&& value) const {
      return boost::python::object{std::forward<U>(value)};
    }
  };
}

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
      const std::shared_ptr<QueueWriter<boost::python::object>>&
        GetTarget() const;

      virtual void Push(const Source& value) override final;

      virtual void Push(Source&& value) override final;

      virtual void Break(const std::exception_ptr& e) override final;

    private:
      template<typename U> friend std::shared_ptr<FromPythonQueueWriter<U>>
        MakeFromPythonQueueWriter(
        std::shared_ptr<QueueWriter<boost::python::object>> target);
      std::shared_ptr<void> m_self;
      std::shared_ptr<QueueWriter<boost::python::object>> m_target;

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
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    m_target.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueWriter<boost::python::object>>&
      FromPythonQueueWriter<T>::GetTarget() const {
    return m_target;
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(const Source& value) {
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    if(m_target == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    try {
      m_target->Push(Details::ToObject<T>{}(value));
    } catch(const std::exception&) {
      m_target = nullptr;
      m_self.reset();
      throw;
    }
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(Source&& value) {
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    if(m_target == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException{});
    }
    try {
      m_target->Push(Details::ToObject<T>{}(std::move(value)));
    } catch(const std::exception&) {
      m_target = nullptr;
      m_self.reset();
      throw;
    }
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    Python::GilLock gil;
    boost::lock_guard<Python::GilLock> lock{gil};
    if(m_target == nullptr) {
      return;
    }
    m_target->Break(e);
    m_target = nullptr;
    m_self.reset();
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Bind(std::shared_ptr<void> self) {
    m_self = std::move(self);
  }
}

#endif
