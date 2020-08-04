#ifndef BEAM_PYTHON_QUEUE_READER_HPP
#define BEAM_PYTHON_QUEUE_READER_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Optional.hpp"
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

    Target Pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(Target, T, "pop", Pop);
    }

    boost::optional<Target> TryPop() override {
      PYBIND11_OVERLOAD_PURE_NAME(Target, T, "try_pop", TryPop);
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

      Target Pop() override;

      boost::optional<Target> TryPop() override;

      void Break(const std::exception_ptr& e) override;

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
    : m_source(std::move(source)) {}

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
  typename FromPythonQueueReader<T>::Target FromPythonQueueReader<T>::Pop() {
    auto value = m_source->TryPop();
    if(!value) {
      auto release = GilRelease();
      value = m_source->Pop();
    }
    auto lock = GilLock();
    return value->cast<Target>();
  }

  template<typename T>
  boost::optional<typename FromPythonQueueReader<T>::Target>
      FromPythonQueueReader<T>::TryPop() {
    if(auto value = m_source->TryPop()) {
      auto lock = GilLock();
      return value->cast<T>();
    }
    return boost::none;
  }

  template<typename T>
  void FromPythonQueueReader<T>::Break(const std::exception_ptr& e) {
    m_source->Break(e);
  }
}

#endif
