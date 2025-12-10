#ifndef BEAM_PYTHON_QUEUE_READER_HPP
#define BEAM_PYTHON_QUEUE_READER_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/ScopedQueueReader.hpp"

namespace Beam::Python {

  /**
   * Provides a trampoline template for exporting QueueReader classes.
   * @tparam T The type of QueueReader to trampoline.
   */
  template<typename T>
  struct TrampolineQueueReader final : T {
    using Source = typename T::Source;
    using T::T;

    Source pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(Source, T, "pop", pop);
    }

    boost::optional<Source> try_pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(
        boost::optional<Source>, T, "try_pop", try_pop);
    }

    void close(const std::exception_ptr& e) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "close", close, e);
    }
  };

  /**
   * Wraps a QueueReader of Python objects to a QueueReader of type T.
   * @tparam T The type of data to pop from the queue.
   */
  template<typename T>
  class FromPythonQueueReader final : public QueueReader<T> {
    public:
      using Source = typename QueueReader<T>::Source;

      /**
       * Constructs a FromPythonQueueReader.
       * @param source The QueueReader to wrap.
       */
      explicit FromPythonQueueReader(
        std::shared_ptr<QueueReader<pybind11::object>> source);

      ~FromPythonQueueReader() override;

      /** Returns the QueueReader being wrapped. */
      const std::shared_ptr<QueueReader<pybind11::object>>& get_source() const;

      Source pop() override;
      boost::optional<Source> try_pop() override;
      void close(const std::exception_ptr& e) override;

    private:
      std::shared_ptr<QueueReader<pybind11::object>> m_source;
  };

  /**
   * Makes a FromPythonQueueReader.
   * @param source The QueueReader to wrap.
   */
  template<typename T>
  auto make_from_python_queue_reader(
      std::shared_ptr<QueueReader<pybind11::object>> source) {
    return std::make_shared<FromPythonQueueReader<T>>(std::move(source));
  }

  /**
   * Provides a caster from a std::shared_ptr<QueueReader<T>> to a
   * ScopedQueueReader<T>.
   */
  template<typename T>
  struct ScopedQueueReaderTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using Converter = pybind11::detail::make_caster<
      std::shared_ptr<QueueReader<typename Type::Source>>>;
    static constexpr auto name = pybind11::detail::_("ScopedQueueReader");
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

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
      FromPythonQueueReader<T>::get_source() const {
    return m_source;
  }

  template<typename T>
  typename FromPythonQueueReader<T>::Source FromPythonQueueReader<T>::pop() {
    if(auto value = try_pop()) {
      return std::move(*value);
    }
    auto value = [&] {
      auto release = GilRelease();
      return m_source->pop();
    }();
    auto lock = GilLock();
    return value.template cast<Source>();
  }

  template<typename T>
  boost::optional<typename FromPythonQueueReader<T>::Source>
      FromPythonQueueReader<T>::try_pop() {
    if(auto value = m_source->try_pop()) {
      auto lock = GilLock();
      return value->template cast<Source>();
    }
    return boost::none;
  }

  template<typename T>
  void FromPythonQueueReader<T>::close(const std::exception_ptr& e) {
    m_source->close(e);
  }

  template<typename T>
  pybind11::handle ScopedQueueReaderTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      std::shared_ptr<QueueReader<typename Type::Source>>>::policy(policy);
    return Converter::cast(std::shared_ptr<QueueReader<typename Type::Source>>(
      std::make_shared<Type>(std::move(value))), policy, parent);
  }

  template<typename T>
  bool ScopedQueueReaderTypeCaster<T>::load(
      pybind11::handle source, bool convert) {
    auto caster = Converter();
    if(!caster.load(source, convert)) {
      return false;
    }
    m_value.emplace(pybind11::detail::cast_op<
      std::shared_ptr<QueueReader<typename Type::Source>>&&>(
        std::move(caster)));
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::ScopedQueueReader<T>> :
    Beam::Python::ScopedQueueReaderTypeCaster<Beam::ScopedQueueReader<T>> {};
}

#endif
