#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/PythonWrapperReactor.hpp"
#include "Beam/Python/Ref.hpp"
#include "Beam/Python/SignalsSlots.hpp"
#include "Beam/Python/Tuple.hpp"
#include "Beam/Reactors/AggregateReactor.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/ChainReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/Expressions.hpp"
#include "Beam/Reactors/FilterReactor.hpp"
#include "Beam/Reactors/FoldReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"
#include "Beam/Reactors/NonRepeatingReactor.hpp"
#include "Beam/Reactors/ProxyReactor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/RangeReactor.hpp"
#include "Beam/Reactors/ReactorError.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/StaticReactor.hpp"
#include "Beam/Reactors/SwitchReactor.hpp"
#include "Beam/Reactors/ThrowReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Reactors/WhenComplete.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/VirtualTimer.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/VirtualTimeClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Reactors;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct BaseReactorWrapper : BaseReactor, wrapper<BaseReactor> {
    virtual bool IsComplete() const override final {
      return this->get_override("is_complete");
    }

    virtual const std::type_info& GetType() const override final {
      return typeid(boost::python::object);
    }

    virtual Update Commit(int sequenceNumber) override final {
      return this->get_override("commit")(sequenceNumber);
    }
  };

  auto MakePythonAggregateReactor(
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>> reactor) {
    return std::static_pointer_cast<PythonReactor>(
      MakeAggregateReactor(std::move(reactor)));
  }

  auto MakePythonAlarmReactor(const object& timerFactory,
      VirtualTimeClient* timeClient,
      const std::shared_ptr<Reactor<ptime>>& expiryReactor,
      Trigger& trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    return std::static_pointer_cast<Reactor<bool>>(
      MakeAlarmReactor(pythonTimerFactory, timeClient, expiryReactor,
      Ref(trigger)));
  }

  auto MakePythonDefaultAlarmReactor(
      const std::shared_ptr<Reactor<ptime>>& expiryReactor, Trigger& trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        auto timer = std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
        return timer;
      };
    return std::static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(
      pythonTimerFactory, std::make_shared<LocalTimeClient>(), expiryReactor,
      Ref(trigger)));
  }

  auto MakePythonChainReactor(std::shared_ptr<PythonReactor> initial,
      std::shared_ptr<PythonReactor> continuation, RefType<Trigger> trigger) {
    return std::static_pointer_cast<Reactor<object>>(
      MakeChainReactor(std::move(initial), std::move(continuation),
      Ref(trigger)));
  }

  auto MakePythonConstantReactor(const boost::python::object& value) {
    return std::static_pointer_cast<PythonReactor>(MakeConstantReactor(value));
  }

  std::shared_ptr<PythonReactor> MakePythonDoReactor(const object& callable,
      const std::shared_ptr<PythonReactor>& reactor) {
    return Do(
      [=] (const Expect<object>& value) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          callable(value);
        } catch(const boost::python::error_already_set&) {
          PrintError();
        }
      }, reactor);
  }

  auto MakePythonNoneReactor() {
    return std::static_pointer_cast<PythonReactor>(
      std::shared_ptr<NoneReactor<object>>());
  }

  std::shared_ptr<PythonReactor> MakePythonNonRepeatingReactor(
      const std::shared_ptr<PythonReactor>& reactor) {
    return MakeNonRepeatingReactor(reactor);
  }

  auto MakePythonRangeReactor(std::shared_ptr<PythonReactor> lower,
      std::shared_ptr<PythonReactor> upper, RefType<Trigger> trigger) {
    return MakePythonWrapperReactor(
      MakeRangeReactor(std::move(lower), std::move(upper), Ref(trigger)));
  }

  auto MakePythonStaticReactor(std::shared_ptr<PythonReactor> source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeStaticReactor(std::move(source)));
  }

  auto MakePythonSwitchReactor(
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>> source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeSwitchReactor(std::move(source)));
  }

  auto MakePythonTimerReactor(const object& timerFactory,
      const std::shared_ptr<Reactor<time_duration>>& periodReactor,
      Trigger* trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        VirtualTimer* result = extract<VirtualTimer*>(timerFactory(duration));
        return MakeVirtualTimer<VirtualTimer*>(std::move(result));
      };
    return MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      periodReactor, Ref(*trigger));
  }

  auto MakePythonDefaultTimerReactor(
      const std::shared_ptr<Reactor<time_duration>>& periodReactor,
      Trigger* trigger) {
    auto pythonTimerFactory =
      [=] (const time_duration& duration) {
        auto timer = std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
        return timer;
      };
    return MakeTimerReactor<std::int64_t>(pythonTimerFactory,
      periodReactor, Ref(*trigger));
  }

  void ReactorMonitorDo(ReactorMonitor* monitor, const object& callable) {
    monitor->Do(
      [=] {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          callable();
        } catch(const boost::python::error_already_set&) {
          PrintError();
        }
      });
  }

  int TriggerSignalUpdate(Trigger* trigger) {
    int sequenceNumber;
    trigger->SignalUpdate(Store(sequenceNumber));
    return sequenceNumber;
  }
}

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile AggregateReactor<
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>*
      get_pointer(const volatile AggregateReactor<
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>* p) {
    return p;
  }

  template<> inline const volatile BaseReactor* get_pointer(
      const volatile BaseReactor* p) {
    return p;
  }

  template<> inline const volatile BaseReactorWrapper* get_pointer(
      const volatile BaseReactorWrapper* p) {
    return p;
  }

  template<> inline const volatile BasicReactor<boost::python::object>*
      get_pointer(const volatile BasicReactor<boost::python::object>* p) {
    return p;
  }

  template<> inline const volatile
      ChainReactor<std::shared_ptr<PythonReactor>,
      std::shared_ptr<PythonReactor>>* get_pointer(
      const volatile ChainReactor<std::shared_ptr<PythonReactor>,
      std::shared_ptr<PythonReactor>>* p) {
    return p;
  }

  template<> inline const volatile ConstantReactor<object>* get_pointer(
      const volatile ConstantReactor<object>* p) {
    return p;
  }

  template<> inline const volatile FoldParameterReactor<object>* get_pointer(
      const volatile FoldParameterReactor<object>* p) {
    return p;
  }

  template<> inline const volatile FoldReactor<std::shared_ptr<PythonReactor>,
    std::shared_ptr<PythonReactor>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<FoldParameterReactor<object>>>* get_pointer(
      const volatile FoldReactor<std::shared_ptr<PythonReactor>,
      std::shared_ptr<PythonReactor>,
      std::shared_ptr<FoldParameterReactor<object>>,
      std::shared_ptr<FoldParameterReactor<object>>>* p) {
    return p;
  }

  template<> inline const volatile NoneReactor<object>* get_pointer(
      const volatile NoneReactor<object>* p) {
    return p;
  }

  template<> inline const volatile ProxyReactor<object>* get_pointer(
      const volatile ProxyReactor<object>* p) {
    return p;
  }

  template<> inline const volatile Publisher<int>* get_pointer(
      const volatile Publisher<int>* p) {
    return p;
  }

  template<> inline const volatile PythonReactor* get_pointer(
      const volatile PythonReactor* p) {
    return p;
  }

  template<> inline const volatile Reactor<bool>* get_pointer(
      const volatile Reactor<bool>* p) {
    return p;
  }

  template<> inline const volatile Reactor<std::int64_t>* get_pointer(
      const volatile Reactor<std::int64_t>* p) {
    return p;
  }

  template<> inline const volatile Reactor<ptime>* get_pointer(
      const volatile Reactor<ptime>* p) {
    return p;
  }

  template<> inline const volatile Reactor<std::shared_ptr<Reactor<object>>>*
      get_pointer(const volatile Reactor<std::shared_ptr<Reactor<object>>>* p) {
    return p;
  }

  template<> inline const volatile Reactor<time_duration>* get_pointer(
      const volatile Reactor<time_duration>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      PythonReactor>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<PythonReactor>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<bool>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<Reactor<bool>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<ptime>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<Reactor<ptime>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::shared_ptr<Reactor<object>>>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::shared_ptr<Reactor<object>>>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<time_duration>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<time_duration>>* p) {
    return p;
  }

  template<> inline const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::int64_t>>* get_pointer(
      const volatile Beam::Python::Details::ReactorWrapper<
      Reactor<std::int64_t>>* p) {
    return p;
  }

  template<> inline const volatile SwitchReactor<std::shared_ptr<
      Reactor<std::shared_ptr<PythonReactor>>>>* get_pointer(const volatile
      SwitchReactor<
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>* p) {
    return p;
  }

  template<> inline const volatile std::type_info* get_pointer(
      const volatile std::type_info* p) {
    return p;
  }
}
#endif

void Beam::Python::ExportAggregateReactor() {
  using ExportedReactor =
    AggregateReactor<std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("AggregateReactor",
    init<const std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>&>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("aggregate", &MakePythonAggregateReactor);
}

void Beam::Python::ExportAlarmReactor() {
  def("AlarmReactor", &MakePythonAlarmReactor);
  def("AlarmReactor", &MakePythonDefaultAlarmReactor);
  def("alarm", &MakePythonAlarmReactor);
  def("alarm", &MakePythonDefaultAlarmReactor);
}

void Beam::Python::ExportBaseReactor() {
  class_<BaseReactorWrapper, std::shared_ptr<BaseReactorWrapper>,
    boost::noncopyable>("BaseReactor")
    .def("is_complete", pure_virtual(&BaseReactor::IsComplete))
    .def("commit", pure_virtual(&BaseReactor::Commit));
  register_ptr_to_python<std::shared_ptr<BaseReactor>>();
  implicitly_convertible<std::shared_ptr<BaseReactorWrapper>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportBasicReactor() {
  using ExportedReactor = BasicReactor<boost::python::object>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("BasicReactor", init<RefType<Trigger>>())
    .def("commit", BlockingFunction(&ExportedReactor::Commit))
    .def("update", &ExportedReactor::Update)
    .def("set_complete", static_cast<void (ExportedReactor::*)()>(
    &ExportedReactor::SetComplete));
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportChainReactor() {
  using ExportedReactor = ChainReactor<std::shared_ptr<PythonReactor>,
    std::shared_ptr<PythonReactor>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("ChainReactor",
    init<std::shared_ptr<PythonReactor>, std::shared_ptr<PythonReactor>,
    RefType<Trigger>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("chain", &MakePythonChainReactor);
}

void Beam::Python::ExportDoReactor() {
  def("do", &MakePythonDoReactor);
}

void Beam::Python::ExportExpressionReactors() {
  def("add", PythonWrapReactor(&Reactors::Add<object, object>));
}

void Beam::Python::ExportFilterReactor() {}

void Beam::Python::ExportFoldReactor() {
  {
    using ExportedReactor = FoldParameterReactor<object>;
    class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
      std::shared_ptr<ExportedReactor>>("FoldParameterReactor", init<>());
    implicitly_convertible<std::shared_ptr<ExportedReactor>,
      std::shared_ptr<PythonReactor>>();
    implicitly_convertible<std::shared_ptr<ExportedReactor>,
      std::shared_ptr<BaseReactor>>();
  }
  using ExportedReactor = FoldReactor<std::shared_ptr<PythonReactor>,
    std::shared_ptr<PythonReactor>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<FoldParameterReactor<object>>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("FoldReactor",
    init<std::shared_ptr<PythonReactor>, std::shared_ptr<PythonReactor>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<FoldParameterReactor<object>>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportNoneReactor() {
  using ExportedReactor = NoneReactor<boost::python::object>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("NoneReactor", init<>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("none", &MakePythonNoneReactor);
}

void Beam::Python::ExportNonRepeatingReactor() {
  def("NonRepeatingReactor", &MakePythonNonRepeatingReactor);
  def("non_repeating", &MakePythonNonRepeatingReactor);
}

void Beam::Python::ExportProxyReactor() {
  using ExportedReactor = ProxyReactor<object>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("ProxyReactor", init<>())
    .def(init<std::shared_ptr<PythonReactor>>())
    .def("set_reactor", &ExportedReactor::SetReactor)
    .def("commit", BlockingFunction(&ExportedReactor::Commit));
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportPublisherReactor() {}

void Beam::Python::ExportPythonConstantReactor() {
  class_<ConstantReactor<object>, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ConstantReactor<object>>>("ConstantReactor",
    init<const object&>());
  implicitly_convertible<std::shared_ptr<ConstantReactor<object>>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ConstantReactor<object>>,
    std::shared_ptr<BaseReactor>>();
  def("constant", &MakePythonConstantReactor);
}

void Beam::Python::ExportQueueReactor() {}

void Beam::Python::ExportRangeReactor() {
  def("RangeReactor", &MakePythonRangeReactor);
  def("range", &MakePythonRangeReactor);
}

void Beam::Python::ExportReactorMonitor() {
  class_<ReactorMonitor, noncopyable>("ReactorMonitor", init<>())
    .add_property("trigger", make_function(
      static_cast<Trigger& (ReactorMonitor::*)()>(&ReactorMonitor::GetTrigger),
      return_internal_reference<>()))
    .def("add", BlockingFunction(&ReactorMonitor::Add))
    .def("do", &ReactorMonitorDo)
    .def("open", BlockingFunction(&ReactorMonitor::Open))
    .def("close", BlockingFunction(&ReactorMonitor::Close));
}

void Beam::Python::ExportReactors() {
  string nestedName = extract<string>(scope().attr("__name__") + ".reactors");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("reactors") = nestedModule;
  scope parent = nestedModule;
  ExportBaseReactor();
  ExportReactor<PythonReactor>("Reactor");
  ExportReactorMonitor();
  ExportAggregateReactor();
  ExportDoReactor();
  ExportAlarmReactor();
  ExportBasicReactor();
  ExportChainReactor();
  ExportExpressionReactors();
  ExportFilterReactor();
  ExportFoldReactor();
  ExportNoneReactor();
  ExportNonRepeatingReactor();
  ExportProxyReactor();
  ExportPublisherReactor();
  ExportQueueReactor();
  ExportRangeReactor();
  ExportStaticReactor();
  ExportSwitchReactor();
  ExportThrowReactor();
  ExportTimerReactor();
  ExportWhenCompleteReactor();
  ExportTrigger();
  ExportPythonConstantReactor();
  ExportReactor<Reactor<std::shared_ptr<PythonReactor>>>("MetaReactor");
  ExportReactor<Reactor<ptime>>("DateReactor");
  ExportReactor<Reactor<bool>>("BoolReactor");
  ExportReactor<Reactor<time_duration>>("TimeDurationReactor");
  ExportReactor<Reactor<std::int64_t>>("Int64Reactor");
  ExportException<ReactorException, std::runtime_error>("ReactorException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<ReactorError, ReactorException>("ReactorError")
    .def(init<>())
    .def(init<const string&>());
  ExportException<ReactorUnavailableException, ReactorException>(
    "ReactorUnavailableException")
    .def(init<>())
    .def(init<const string&>());
  ExportRef<RefType<Trigger>>("TriggerRef");
  ExportRef<RefType<Reactor<object>>>("PythonReactorRef");
}

void Beam::Python::ExportStaticReactor() {
  def("StaticReactor", &MakePythonStaticReactor);
  def("static", &MakePythonStaticReactor);
}

void Beam::Python::ExportSwitchReactor() {
  using ExportedReactor = SwitchReactor<
    std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("SwitchReactor",
    init<std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("switch", &MakePythonSwitchReactor);
}

void Beam::Python::ExportThrowReactor() {}

void Beam::Python::ExportTimerReactor() {
  def("TimerReactor", &MakePythonTimerReactor);
  def("TimerReactor", &MakePythonDefaultTimerReactor);
  def("timer", &MakePythonTimerReactor);
  def("timer", &MakePythonDefaultTimerReactor);
}

void Beam::Python::ExportTrigger() {
  class_<Trigger, noncopyable>("Trigger", init<>())
    .def("signal_update", &TriggerSignalUpdate)
    .add_property("sequence_number_publisher", make_function(
      &Trigger::GetSequenceNumberPublisher, return_internal_reference<>()));
}

void Beam::Python::ExportWhenCompleteReactor() {}
