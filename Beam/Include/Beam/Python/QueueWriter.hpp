#ifndef BEAM_PYTHON_QUEUE_WRITER_HPP
#define BEAM_PYTHON_QUEUE_WRITER_HPP
#include <type_traits>
#include <boost/throw_exception.hpp>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Queues/WeakQueueWriter.hpp"

namespace Beam::Python {
namespace Details {
  template<typename T, typename Enabled = void>
  struct Extractor;

  template<typename T>
  struct Extractor<T, typename std::enable_if_t<
      std::is_constructible_v<T, pybind11::object>>> {
    auto operator ()(pybind11::object value) {
      return T(std::move(value));
    }
  };

  template<typename T, typename Enabled>
  struct Extractor {
    auto operator ()(const pybind11::object& value) {
      return value.cast<T>();
    }
  };
}

  /**
   * Provides a trampoline template for exporting QueueWriter classes.
   * @param <T> The type of QueueWriter to trampoline.
   */
  template<typename T>
  struct TrampolineQueueWriter final : T {
    using Target = typename T::Target;
    using T::T;

    void Push(Target&& value) override {
      Push(value);
    }

    void Push(const Target& value) override {
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
      using Target = typename QueueWriter<T>::Target;

      /**
       * Constructs a FromPythonQueueWriter.
       * @param target The QueueWriter to wrap.
       */
      FromPythonQueueWriter(
        std::shared_ptr<QueueWriter<pybind11::object>> target, Guard);

      ~FromPythonQueueWriter() override;

      //! Returns the QueueWriter being wrapped.
      const std::shared_ptr<QueueWriter<pybind11::object>>& GetTarget() const;

      void Push(const Target& value) override;

      void Push(Target&& value) override;

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

  /**
   * Wraps a QueueWriter of type T to a QueueWriter of Python objects.
   * @param <T> The type of data to push onto the queue.
   */
  template<typename T>
  class ToPythonQueueWriter final : public QueueWriter<pybind11::object> {
    public:
      using Target = typename QueueWriter<pybind11::object>::Target;

      using Type = T;

      /**
       * Constructs a ToPythonQueueWriter.
       * @param target The QueueWriter to wrap.
       */
      ToPythonQueueWriter(ScopedQueueWriter<Type> target);

      void Push(const Target& value) override;

      void Push(Target&& value) override;

      void Break(const std::exception_ptr& e) override;

    private:
      ScopedQueueWriter<Type> m_target;
  };

  /**
   * Returns a QueueWriter that converts Python objects into objects of an
   * underlying QueueWriter's type.
   */
  template<typename T, typename Q>
  auto MakeToPythonQueueWriter(ScopedQueueWriter<T, Q> target) {
    return std::static_pointer_cast<QueueWriter<pybind11::object>>(
      std::make_shared<ToPythonQueueWriter<T>>(std::move(target)));
  }

  /**
   * Provides a caster from a std::shared_ptr<QueueWriter<T>> to a
   * ScopedQueueWriter<T>.
   */
  template<typename T>
  struct ScopedQueueWriterTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    using Converter = pybind11::detail::make_caster<
      std::shared_ptr<QueueWriter<typename Type::Target>>>;
    static constexpr auto name = pybind11::detail::_("ScopedQueueWriter");
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  FromPythonQueueWriter<T>::FromPythonQueueWriter(
    std::shared_ptr<QueueWriter<pybind11::object>> target, Guard)
    : m_target(std::move(target)) {}

  template<typename T>
  FromPythonQueueWriter<T>::~FromPythonQueueWriter() {
    auto lock = GilLock();
    m_target.reset();
  }

  template<typename T>
  const std::shared_ptr<QueueWriter<pybind11::object>>&
      FromPythonQueueWriter<T>::GetTarget() const {
    return m_target;
  }

  template<typename T>
  void FromPythonQueueWriter<T>::Push(const Target& value) {
    auto lock = GilLock();
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
  void FromPythonQueueWriter<T>::Push(Target&& value) {
    auto lock = GilLock();
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
    auto lock = GilLock();
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

  template<typename T>
  ToPythonQueueWriter<T>::ToPythonQueueWriter(ScopedQueueWriter<Type> target)
    : m_target(std::move(target)) {}

  template<typename T>
  void ToPythonQueueWriter<T>::Push(const Target& value) {
    m_target.Push(Details::Extractor<Type>()(value));
  }

  template<typename T>
  void ToPythonQueueWriter<T>::Push(Target&& value) {
    m_target.Push(Details::Extractor<Type>()(std::move(value)));
  }

  template<typename T>
  void ToPythonQueueWriter<T>::Break(const std::exception_ptr& e) {
    m_target.Break(e);
  }

  template<typename T>
  pybind11::handle ScopedQueueWriterTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    policy = pybind11::detail::return_value_policy_override<
      std::shared_ptr<QueueWriter<typename Type::Target>>>::policy(policy);
    return Converter::cast(std::shared_ptr<QueueWriter<typename Type::Target>>(
      std::make_shared<Type>(std::move(value))), policy, parent);
  }

  template<typename T>
  bool ScopedQueueWriterTypeCaster<T>::load(pybind11::handle source,
      bool convert) {
    auto caster = Converter();
    if(!caster.load(source, convert)) {
      return false;
    }
    m_value.emplace(MakeWeakQueueWriter(pybind11::detail::cast_op<
      std::shared_ptr<QueueWriter<typename Type::Target>>&&>(
      std::move(caster))));
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::ScopedQueueWriter<T>> :
    Beam::Python::ScopedQueueWriterTypeCaster<Beam::ScopedQueueWriter<T>> {};
}

#endif
