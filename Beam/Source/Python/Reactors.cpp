#include "Beam/Python/Reactors.hpp"
#include <boost/preprocessor/comma.hpp>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/NoThrowFunction.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/PythonFunctionReactor.hpp"
#include "Beam/Python/PythonQueueReactor.hpp"
#include "Beam/Python/Ref.hpp"
#include "Beam/Python/SignalsSlots.hpp"
#include "Beam/Python/Tuple.hpp"
#include "Beam/Reactors/AggregateReactor.hpp"
#include "Beam/Reactors/AlarmReactor.hpp"
#include "Beam/Reactors/BaseReactor.hpp"
#include "Beam/Reactors/BasicReactor.hpp"
#include "Beam/Reactors/ChainReactor.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/CurrentTimeReactor.hpp"
#include "Beam/Reactors/DoReactor.hpp"
#include "Beam/Reactors/Expressions.hpp"
#include "Beam/Reactors/FilterReactor.hpp"
#include "Beam/Reactors/FirstReactor.hpp"
#include "Beam/Reactors/FoldReactor.hpp"
#include "Beam/Reactors/LastReactor.hpp"
#include "Beam/Reactors/NoneReactor.hpp"
#include "Beam/Reactors/NonRepeatingReactor.hpp"
#include "Beam/Reactors/ProxyReactor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/QueryReactor.hpp"
#include "Beam/Reactors/RangeReactor.hpp"
#include "Beam/Reactors/ReactorError.hpp"
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/SwitchReactor.hpp"
#include "Beam/Reactors/ThrowReactor.hpp"
#include "Beam/Reactors/TimerReactor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/Reactors/UpdateReactor.hpp"
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
    virtual const std::type_info& GetType() const override final {
      return typeid(boost::python::object);
    }

    virtual Update Commit(int sequenceNumber) override final {
      return get_override("commit")(sequenceNumber);
    }
  };

  struct ApplyFunction {
    boost::python::object m_callable;

    boost::python::object operator ()(const boost::python::tuple& args,
        const boost::python::dict& kw) {
      std::vector<boost::python::object> parameters;
      for(int i = 0; i < boost::python::len(args); ++i) {
        parameters.push_back(args[i].attr("value"));
      }
      auto t = PyTuple_New(static_cast<Py_ssize_t>(parameters.size()));
      for(std::size_t i = 0; i < parameters.size(); ++i) {
        PyTuple_SET_ITEM(t, i, boost::python::incref(parameters[i].ptr()));
      }
      boost::python::object parameterTuple{boost::python::handle<>{
        boost::python::borrowed(t)}};
      auto rawResult = PyObject_Call(m_callable.ptr(), parameterTuple.ptr(),
        kw.ptr());
      if(rawResult == nullptr) {
        PrintError();
        PyErr_Clear();
        return boost::python::object{};
      }
      boost::python::object result{boost::python::handle<>{
        boost::python::borrowed(rawResult)}};
      return result;
    }
  };

  auto ApplyFunctionReactor(const boost::python::tuple& args,
      const boost::python::dict& kw) {
    auto callable = args[0];
    auto parameters = boost::python::tuple{args.slice(1, boost::python::_)};
    return boost::python::object{std::static_pointer_cast<PythonReactor>(
      std::make_shared<PythonFunctionReactor>(raw_function(
      ApplyFunction{callable}), parameters, kw))};
  }

  void DeleteReactorMonitor(ReactorMonitor& monitor) {
    GilRelease gil;
    boost::lock_guard<GilRelease> lock{gil};
    monitor.Close();
  }

  auto MakePythonAggregateReactor(
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>> reactor) {
    return std::static_pointer_cast<PythonReactor>(
      MakeAggregateReactor(std::move(reactor)));
  }

  auto MakePythonAlarmReactor(VirtualTimeClient* timeClient,
      const object& timerFactory,
      const boost::python::object& expiryReactor) {
    auto properTimerFactory =
      [timerFactory = SharedObject{timerFactory}]
          (const time_duration& expiry) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        return MakeVirtualTimer(extract<std::shared_ptr<VirtualTimer>>{
          (*timerFactory)(expiry)}());
      };
    return std::static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(timeClient,
      properTimerFactory, ExtractReactor<ptime>(expiryReactor)));
  }

  auto MakePythonDefaultAlarmReactor(
      const boost::python::object& expiryReactor) {
    auto timerFactory =
      [] (const time_duration& duration) {
        return std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
      };
    return std::static_pointer_cast<Reactor<bool>>(MakeAlarmReactor(
      std::make_shared<LocalTimeClient>(), timerFactory,
      ExtractReactor<ptime>(expiryReactor)));
  }

  auto MakePythonChainReactor(const boost::python::object& initial,
      const boost::python::object& continuation) {
    return std::static_pointer_cast<Reactor<object>>(MakeChainReactor(
      ExtractReactor(initial), ExtractReactor(continuation)));
  }

  auto MakePythonConstantReactor(const boost::python::object& value) {
    return std::static_pointer_cast<PythonReactor>(MakeConstantReactor(value));
  }

  auto MakePythonCurrentTimeReactor(VirtualTimeClient* timeClient) {
    return std::static_pointer_cast<PythonReactor>(
      MakeToPythonReactor(static_pointer_cast<Reactor<ptime>>(
      MakeCurrentTimeReactor(timeClient))));
  }

  auto MakePythonDefaultCurrentTimeReactor() {
    return std::static_pointer_cast<PythonReactor>(
      MakeToPythonReactor(static_pointer_cast<Reactor<ptime>>(
      MakeCurrentTimeReactor())));
  }

  auto MakePythonDoReactor(
      const NoThrowFunction<void, const Expect<object>&>& callback,
      const boost::python::object& reactor) {
    return std::static_pointer_cast<PythonReactor>(
      Do(callback, ExtractReactor(reactor)));
  }

  auto MakePythonFilterReactor(const boost::python::object& filter,
      const boost::python::object& source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeFilterReactor(ExtractReactor<bool>(filter), ExtractReactor(source)));
  }

  auto MakePythonFirstReactor(const boost::python::object& source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeFirstReactor(ExtractReactor(source)));
  }

  auto MakePythonFoldReactor(const boost::python::object& f,
      const boost::python::object& source) {
    auto leftOperand = MakeFoldParameterReactor<object>();
    auto rightOperand = MakeFoldParameterReactor<object>();
    auto fold = extract<std::shared_ptr<PythonReactor>>{ApplyFunctionReactor(
      boost::python::make_tuple(f,
      std::static_pointer_cast<PythonReactor>(leftOperand),
      std::static_pointer_cast<PythonReactor>(rightOperand)),
      boost::python::dict{})}();
    return std::static_pointer_cast<PythonReactor>(MakeFoldReactor(fold,
      leftOperand, rightOperand, ExtractReactor(source)));
  }

  boost::python::object MakePythonFunctionReactor(
      const boost::python::tuple& args, const boost::python::dict& kw) {
    auto self = args[0];
    auto callable = args[1];
    auto a = boost::python::tuple{args.slice(2, boost::python::_)};
    return self.attr("__init__")(callable, a, kw);
  }

  auto MakePythonLastReactor(const boost::python::object& source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeLastReactor(ExtractReactor(source)));
  }

  auto MakePythonNoneReactor() {
    return std::static_pointer_cast<PythonReactor>(
      std::shared_ptr<NoneReactor<object>>());
  }

  auto MakePythonPublisherReactor(const Publisher<object>& publisher) {
    return std::static_pointer_cast<PythonReactor>(
      MakePublisherReactor(publisher));
  }

  auto MakePythonNonRepeatingReactor(const boost::python::object& reactor) {
    return std::static_pointer_cast<PythonReactor>(
      MakeNonRepeatingReactor(ExtractReactor(reactor)));
  }

  auto MakePythonQueryReactor(boost::python::object submissionFunction,
      const Queries::BasicQuery<object>& query) {
    return std::static_pointer_cast<PythonReactor>(
      MakeQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, query));
  }

  auto MakePythonCurrentQueryReactor(boost::python::object submissionFunction,
      const object& index) {
    return std::static_pointer_cast<PythonReactor>(
      MakeCurrentQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, index));
  }

  auto MakePythonRealTimeQueryReactor(boost::python::object submissionFunction,
      const object& index) {
    return std::static_pointer_cast<PythonReactor>(
      MakeRealTimeQueryReactor<boost::python::object>(
        [submissionFunction] (const Queries::BasicQuery<object>& query,
            std::shared_ptr<QueueWriter<object>> queue) {
          try {
            submissionFunction(query, queue);
          } catch(const boost::python::error_already_set&) {
            PrintError();
          }
        }, index));
  }

  auto MakePythonQueueReactor(std::shared_ptr<QueueReader<object>> queue) {
    return std::static_pointer_cast<PythonReactor>(MakeQueueReactor(queue));
  }

  auto MakePythonRangeReactor(
      const boost::python::object& lower, const boost::python::object& upper) {
    return std::static_pointer_cast<PythonReactor>(MakeRangeReactor(
      ExtractReactor(lower), ExtractReactor(upper)));
  }

  auto MakePythonSwitchReactor(
      std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>> source) {
    return std::static_pointer_cast<PythonReactor>(
      MakeSwitchReactor(std::move(source)));
  }

  auto MakePythonThrowReactor(const ReactorException& e) {
    return std::static_pointer_cast<PythonReactor>(MakeThrowReactor<object>(e));
  }

  auto MakePythonTimerReactor(const object& timerFactory,
      const boost::python::object& periodReactor) {
    auto properTimerFactory =
      [timerFactory = SharedObject{timerFactory}]
          (const time_duration& expiry) {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        return MakeVirtualTimer(extract<std::shared_ptr<VirtualTimer>>{
          (*timerFactory)(expiry)}());
      };
    return std::static_pointer_cast<PythonReactor>(MakeToPythonReactor(
      std::static_pointer_cast<Reactor<std::int64_t>>(
      MakeTimerReactor<std::int64_t>(properTimerFactory,
      ExtractReactor<time_duration>(periodReactor)))));
  }

  auto MakePythonDefaultTimerReactor(
      const boost::python::object& periodReactor) {
    auto timerFactory =
      [=] (const time_duration& duration) {
        return std::make_unique<LiveTimer>(duration,
          Ref(*GetTimerThreadPool()));
      };
    return std::static_pointer_cast<PythonReactor>(MakeToPythonReactor(
      std::static_pointer_cast<Reactor<std::int64_t>>(
      MakeTimerReactor<std::int64_t>(timerFactory,
      ExtractReactor<time_duration>(periodReactor)))));
  }

  auto MakeWhenCompleteReactor(const NoThrowFunction<void>& callback,
      const boost::python::object& reactor) {
    return std::static_pointer_cast<PythonReactor>(
      WhenComplete(callback, ExtractReactor(reactor)));
  }

  void ReactorMonitorAdd(ReactorMonitor& monitor,
      std::shared_ptr<BaseReactor> reactor) {
    monitor.Add(MakeFromPythonReactor(std::move(reactor)));
  }

  void ReactorMonitorDo(ReactorMonitor& monitor,
      const NoThrowFunction<void>& f) {
    monitor.Do(f);
  }

  int TriggerSignalUpdate(Trigger& trigger) {
    int sequenceNumber;
    trigger.SignalUpdate(Store(sequenceNumber));
    return sequenceNumber;
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(
  AggregateReactor<std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(BaseReactor);
BEAM_DEFINE_PYTHON_POINTER_LINKER(BaseReactorWrapper);
BEAM_DEFINE_PYTHON_POINTER_LINKER(BasicReactor<boost::python::object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ChainReactor<std::shared_ptr<PythonReactor> BOOST_PP_COMMA()
  std::shared_ptr<PythonReactor>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ConstantReactor<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(FoldParameterReactor<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  FoldReactor<std::shared_ptr<PythonReactor> BOOST_PP_COMMA()
  std::shared_ptr<FoldParameterReactor<object>> BOOST_PP_COMMA()
  std::shared_ptr<FoldParameterReactor<object>> BOOST_PP_COMMA()
  std::shared_ptr<PythonReactor>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(NoneReactor<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ProxyReactor<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Publisher<int>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(PythonReactor);
BEAM_DEFINE_PYTHON_POINTER_LINKER(PythonFunctionReactor);
BEAM_DEFINE_PYTHON_POINTER_LINKER(PythonQueueReactor);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<BaseReactor::Update>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<bool>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<std::int64_t>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<ptime>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<std::shared_ptr<Reactor<object>>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Reactor<time_duration>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Beam::Python::Details::ReactorWrapper<PythonReactor>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Beam::Python::Details::ReactorWrapper<Reactor<BaseReactor::Update>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Beam::Python::Details::ReactorWrapper<Reactor<bool>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Beam::Python::Details::ReactorWrapper<Reactor<ptime>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Python::Details::ReactorWrapper<
  Reactor<std::shared_ptr<Reactor<object>>>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Python::Details::ReactorWrapper<
  Reactor<time_duration>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Beam::Python::Details::ReactorWrapper<
  Reactor<std::int64_t>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(SwitchReactor<
  std::shared_ptr<Reactor<std::shared_ptr<PythonReactor>>>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(ThrowReactor<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(UpdateReactor);
BEAM_DEFINE_PYTHON_POINTER_LINKER(WhenCompleteReactor<
  NoThrowFunction<void> BOOST_PP_COMMA() std::shared_ptr<PythonReactor>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(std::type_info);

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
    .def("commit", pure_virtual(&BaseReactor::Commit));
  register_ptr_to_python<std::shared_ptr<BaseReactor>>();
  implicitly_convertible<std::shared_ptr<BaseReactorWrapper>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportBasicReactor() {
  using ExportedReactor = BasicReactor<boost::python::object>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("BasicReactor", init<>())
    .def("update", &ExportedReactor::Update)
    .def("set_complete", static_cast<void (ExportedReactor::*)()>(
    &ExportedReactor::SetComplete))
    .def("set_complete", static_cast<void (ExportedReactor::*)(
    std::exception_ptr)>(&ExportedReactor::SetComplete));
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
    init<std::shared_ptr<PythonReactor>, std::shared_ptr<PythonReactor>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("chain", &MakePythonChainReactor);
}

void Beam::Python::ExportCurrentTimeReactor() {
  def("CurrentTimeReactor", &MakePythonCurrentTimeReactor);
  def("CurrentTimeReactor", &MakePythonDefaultCurrentTimeReactor);
  def("current_time", &MakePythonCurrentTimeReactor);
  def("current_time", &MakePythonDefaultCurrentTimeReactor);
}

void Beam::Python::ExportDoReactor() {
  def("do", &MakePythonDoReactor);
}

void Beam::Python::ExportFilterReactor() {
  def("FilterReactor", &MakePythonFilterReactor);
  def("filter", &MakePythonFilterReactor);
}

void Beam::Python::ExportFirstReactor() {
  def("FirstReactor", &MakePythonFirstReactor);
  def("first", &MakePythonFirstReactor);
}

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
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<PythonReactor>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("FoldReactor",
    init<std::shared_ptr<PythonReactor>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<FoldParameterReactor<object>>,
    std::shared_ptr<PythonReactor>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("fold", &MakePythonFoldReactor);
}

void Beam::Python::ExportFunctionReactor() {
   ExportFunction<NoThrowFunction<void, const Expect<object>&>>(
    "VoidNoThrowExpectObjectFunction");
  class_<PythonFunctionReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<PythonFunctionReactor>>("FunctionReactor", no_init)
    .def("__init__", raw_function(&MakePythonFunctionReactor, 1))
    .def(init<const boost::python::object&, const boost::python::tuple&,
      const boost::python::dict&>());
  implicitly_convertible<std::shared_ptr<PythonFunctionReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<PythonFunctionReactor>,
    std::shared_ptr<BaseReactor>>();
  def("apply", raw_function(&ApplyFunctionReactor, 1));
}

void Beam::Python::ExportLastReactor() {
  def("LastReactor", &MakePythonLastReactor);
  def("last", &MakePythonLastReactor);
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
    .def("set_reactor", &ExportedReactor::SetReactor);
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
}

void Beam::Python::ExportPublisherReactor() {
  def("PublisherReactor", &MakePythonPublisherReactor);
  def("publisher", &MakePythonPublisherReactor);
}

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

void Beam::Python::ExportQueryReactor() {
  ExportFunction<NoThrowFunction<void, const Queries::BasicQuery<object>&,
    std::shared_ptr<QueueWriter<object>>>>("QuerySubmissionFunction");
  def("QueryReactor", &MakePythonQueryReactor);
  def("query", &MakePythonQueryReactor);
  def("query_current", &MakePythonCurrentQueryReactor);
  def("query_real_time", &MakePythonRealTimeQueryReactor);
}

void Beam::Python::ExportQueueReactor() {
  using ExportedReactor = PythonQueueReactor;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("QueueReactor",
    init<std::shared_ptr<QueueReader<object>>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("queue", &MakePythonQueueReactor);
}

void Beam::Python::ExportRangeReactor() {
  def("RangeReactor", &MakePythonRangeReactor);
  def("range", &MakePythonRangeReactor);
}

void Beam::Python::ExportReactorMonitor() {
  class_<ReactorMonitor, noncopyable>("ReactorMonitor", init<>())
    .def("__del__", &DeleteReactorMonitor)
    .add_property("trigger", make_function(
      static_cast<Trigger& (ReactorMonitor::*)()>(&ReactorMonitor::GetTrigger),
      return_internal_reference<>()))
    .def("add", &ReactorMonitorAdd)
    .def("do", &ReactorMonitorDo)
    .def("wait", BlockingFunction(&ReactorMonitor::Wait))
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
  ExportCurrentTimeReactor();
  ExportFilterReactor();
  ExportFirstReactor();
  ExportFoldReactor();
  ExportFunctionReactor();
  ExportLastReactor();
  ExportNoneReactor();
  ExportNonRepeatingReactor();
  ExportProxyReactor();
  ExportPublisherReactor();
  ExportQueryReactor();
  ExportQueueReactor();
  ExportRangeReactor();
  ExportSwitchReactor();
  ExportThrowReactor();
  ExportTimerReactor();
  ExportUpdateReactor();
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
  ExportRef<Trigger>("TriggerRef");
  ExportRef<Reactor<object>>("PythonReactorRef");
  ExportRef<ReactorMonitor>("ReactorMonitorRef");
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

void Beam::Python::ExportThrowReactor() {
  class_<ThrowReactor<object>, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ThrowReactor<object>>>("ThrowReactor",
    init<const ReactorException&>());
  implicitly_convertible<std::shared_ptr<ThrowReactor<object>>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ThrowReactor<object>>,
    std::shared_ptr<BaseReactor>>();
  def("throw", &MakePythonThrowReactor);
}

void Beam::Python::ExportTimerReactor() {
  ExportFunction<NoThrowFunction<std::unique_ptr<VirtualTimer>,
    const time_duration&>>("TimerNoThrowTimeDurationFunction");
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

void Beam::Python::ExportUpdateReactor() {
  ExportReactor<Reactor<BaseReactor::Update>>("BaseReactorUpdateReactor");
  class_<UpdateReactor, bases<Reactor<BaseReactor::Update>>, boost::noncopyable,
    std::shared_ptr<UpdateReactor>>("UpdateReactor",
    init<std::shared_ptr<BaseReactor>>());
  implicitly_convertible<std::shared_ptr<UpdateReactor>,
    std::shared_ptr<Reactor<BaseReactor::Update>>>();
  implicitly_convertible<std::shared_ptr<UpdateReactor>,
    std::shared_ptr<BaseReactor>>();
  def("on_update", &MakeUpdateReactor);
}

void Beam::Python::ExportWhenCompleteReactor() {
  using Callback = NoThrowFunction<void>;
  using ExportedReactor = WhenCompleteReactor<Callback,
    std::shared_ptr<PythonReactor>>;
  class_<ExportedReactor, bases<PythonReactor>, boost::noncopyable,
    std::shared_ptr<ExportedReactor>>("WhenCompleteReactor",
    init<const Callback&, std::shared_ptr<PythonReactor>>());
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<PythonReactor>>();
  implicitly_convertible<std::shared_ptr<ExportedReactor>,
    std::shared_ptr<BaseReactor>>();
  def("when_complete", &MakeWhenCompleteReactor);
}
