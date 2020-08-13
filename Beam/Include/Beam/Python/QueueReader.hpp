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
    using Source = typename T::Source;
    using T::T;

    Source Pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(Source, T, "pop", Pop);
    }

    boost::optional<Source> TryPop() override {
      PYBIND11_OVERLOAD_PURE_NAME(boost::optional<Source>, T, "try_pop",
        TryPop);
    }
  };

  /**
   * Wraps a QueueReader of Python objects to a QueueReader of type T.
   * @param <T> The type of data to pop from the queue.
   */
  template<typename T>
  class FromPythonQueueReader final : public QueueReader<T> {
    public:
      using Source = typename QueueReader<T>::Source;

      /**
       * Constructs a FromPythonQueueReader.
       * @param source The QueueReader to wrap.
       */
      FromPythonQueueReader(std::shared_ptr<QueueReader<pybind11::object>>
        source);

      ~FromPythonQueueReader() override;

      //! Returns the QueueReader being wrapped.
      const std::shared_ptr<QueueReader<pybind11::object>>& GetSource() const;

      Source Pop() override;

      boost::optional<Source> TryPop() override;

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
  typename FromPythonQueueReader<T>::Source FromPythonQueueReader<T>::Pop() {
    if(auto value = TryPop()) {
      return std::move(*value);
    }
    auto value = [&] {
      auto release = GilRelease();
      return m_source->Pop();
    }();
    auto lock = GilLock();
    return value.template cast<Source>();
  }

  template<typename T>
  boost::optional<typename FromPythonQueueReader<T>::Source>
      FromPythonQueueReader<T>::TryPop() {
    if(auto value = m_source->TryPop()) {
      auto lock = GilLock();
      return value->template cast<Source>();
    }
    return boost::none;
  }

  template<typename T>
  void FromPythonQueueReader<T>::Break(const std::exception_ptr& e) {
    m_source->Break(e);
  }
}

#endif
