#ifndef BEAM_REACTORS_HPP
#define BEAM_REACTORS_HPP

namespace Beam {
namespace Reactors {
  template<typename ProducerReactorType> class AggregateReactor;
  class BaseReactor;
  template<typename InitialReactorType, typename ContinuationReactorType>
    class ChainReactor;
  template<typename T> class ConstantReactor;
  class Event;
  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftTriggerReactorType, typename RightTriggerReactorType>
    class FoldReactor;
  template<typename FunctionType, typename... ParameterTypes>
    class FunctionReactor;
  class LuaReactorParameter;
  template<typename FunctionType, typename ParameterType> class MultiReactor;
  template<typename T> class NativeLuaReactorParameter;
  template<typename T> class NoneReactor;
  template<typename PublisherType> class PublisherReactor;
  template<typename T> class Reactor;
  template<typename ReactorType> class ReactorContainer;
  class ReactorError;
  class ReactorException;
  class ReactorMonitor;
  class ReactorUnavailableException;
  template<typename ProducerReactorType> class SwitchReactor;
  class Trigger;
  template<typename T> class TriggeredReactor;
  template<typename ReactorType> class WeakReactor;
}
}

#endif
