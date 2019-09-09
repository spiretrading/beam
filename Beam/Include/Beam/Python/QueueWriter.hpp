#ifndef BEAM_PYTHON_QUEUE_WRITER_HPP
#define BEAM_PYTHON_QUEUE_WRITER_HPP
#include <boost/throw_exception.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam::Python {

  /**
   * Provides a trampoline template for exporting QueueWriter classes.
   * @param <T> The type of QueueWriter to trampoline.
   */
  template<typename T>
  struct TrampolineQueueWriter final : T {
    using Source = typename T::Source;
    using T::T;

    void Push(Source&& value) override {
      Push(value);
    }

    void Push(const Source& value) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "push", Push, value);
    }
  };

  /**
   * Wraps a QueueWriter of Python objects to a QueueWriter of type T.
   * @param <T> The type of data to push onto the queue.
   */
  template<typename T>
  class FromPythonQueueWriter final : public QueueWriter<T> {
    private:
      struct Guard {};

    public:
      using Source = typename QueueWriter<T>::Source;

      /**
       * Constructs a FromPythonQueueWriter.
       * @param target The QueueWriter to wrap.
       */
      FromPythonQueueWriter(
        std::shared_ptr<QueueWriter<pybind11::object>> target, Guard);

      ~FromPythonQueueWriter() override;

      //! Returns the QueueWriter being wrapped.
      const std::shared_ptr<QueueWriter<pybind11::object>>& GetTarget() const;

      void Push(const Source& value) override;

      void Push(Source&& value) override;

      void Break(const std::exception_ptr& e) override;

    private:
      template<typename U> friend std::shared_ptr<FromPythonQueueWriter<U>>
        MakeFromPythonQueueWriter(
        std::shared_ptr<QueueWriter<pybind11::object>> target);

      std::shared_ptr<void> m_self;
      std::shared_ptr<QueueWriter<pybind11::object>> m_target;

      void Bind(std::shared_ptr<void> self);
  };

  /**
   * Constructs a FromPythonQueueWriter.
   * @param target The QueueWriter to wrap.
   */
  template<typename T>
  std::shared_ptr<FromPythonQueueWriter<T>> MakeFromPythonQueueWriter(
      std::shared_ptr<QueueWriter<pybind11::object>> target) {
    auto queue = std::make_shared<FromPythonQueueWriter<T>>(std::move(target),
      typename FromPythonQueueWriter<T>::Guard());
    queue->Bind(queue);
    return queue;
  }

  template<typename T>
  FromPythonQueueWriter<T>::FromPythonQueueWriter(
    std::shared_ptr<QueueWriter<pybind11::object>> target, Guard)
    : m_target(std::move(target)) {}

  template<typename T>
  FromPythonQueueWriter<T>::~FromPythonQueueWriter() {
    auto lock = pybind11::gil_scoped_acquire();
    m_target.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueWriter<pybind11::object>>&
      FromPythonQueueWriter<T>::GetTarget() const {
    return m_target;
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(const Source& value) {
    auto lock = pybind11::gil_scoped_acquire();
    if(m_target == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException());
    }
    try {
      m_target->Push(pybind11::cast(value));
    } catch(const std::exception&) {
      m_target = nullptr;
      m_self.reset();
      throw;
    }
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(Source&& value) {
    auto lock = pybind11::gil_scoped_acquire();
    if(m_target == nullptr) {
      BOOST_THROW_EXCEPTION(PipeBrokenException());
    }
    try {
      m_target->Push(pybind11::cast(std::move(value)));
    } catch(const std::exception&) {
      m_target = nullptr;
      m_self.reset();
      throw;
    }
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    auto lock = pybind11::gil_scoped_acquire();
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
