#ifndef BEAM_PYTHON_QUEUE_READER_HPP
#define BEAM_PYTHON_QUEUE_READER_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Queues/QueueReader.hpp"

namespace Beam::Python {

  /**
   * Provides a trampoline template for exporting QueueReader classes.
   * @param <T> The type of QueueReader to trampoline.
   */
  template<typename T>
  struct TrampolineQueueReader final : T {
    using Target = typename T::Target;
    using T::T;

    bool IsEmpty() const override {
      PYBIND11_OVERLOAD_PURE_NAME(bool, T, "is_empty", IsEmpty);
    }

    Target Top() const override {
      PYBIND11_OVERLOAD_PURE_NAME(Target, T, "top", Top);
    }

    void Pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "pop", Pop);
    }
  };

  /**
   * Wraps a QueueReader of Python objects to a QueueReader of type T.
   * @param <T> The type of data to pop from the queue.
   */
  template<typename T>
  class FromPythonQueueReader final : public QueueReader<T> {
    public:
      using Target = typename QueueReader<T>::Target;

      /**
       * Constructs a FromPythonQueueReader.
       * @param source The QueueReader to wrap.
       */
      FromPythonQueueReader(std::shared_ptr<QueueReader<pybind11::object>>
        source);

      ~FromPythonQueueReader() override;

      //! Returns the QueueReader being wrapped.
      const std::shared_ptr<QueueReader<pybind11::object>>& GetSource() const;

      bool IsEmpty() const override;

      Target Top() const override;

      void Pop() override;

      void Break(const std::exception_ptr& e) override;

    protected:
      bool IsAvailable() const override;

    private:
      std::shared_ptr<QueueReader<pybind11::object>> m_source;
  };

  /**
   * Makes a FromPythonQueueReader.
   * @param source The QueueReader to wrap.
   */
  template<typename T>
  auto MakeFromPythonQueueReader(
      std::shared_ptr<QueueReader<pybind11::object>> source) {
    return std::make_shared<FromPythonQueueReader<T>>(std::move(source));
  }

  template<typename T>
  FromPythonQueueReader<T>::FromPythonQueueReader(
    std::shared_ptr<QueueReader<pybind11::object>> source)
    : m_source{std::move(source)} {}

  template<typename T>
  FromPythonQueueReader<T>::~FromPythonQueueReader() {
    auto lock = GilLock();
    m_source.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueReader<pybind11::object>>&
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
      auto release = GilRelease();
      m_source->Wait();
    }
    auto lock = GilLock();
    return m_source->Top().cast<T>();
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
