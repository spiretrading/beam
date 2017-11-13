#ifndef BEAM_REACTORS_HPP
#define BEAM_REACTORS_HPP

namespace Beam {
namespace Reactors {
  template<typename ProducerReactorType> class AggregateReactor;
  class BaseReactor;
  template<typename T> class BasicReactor;
  template<typename InitialReactorType, typename ContinuationReactorType>
    class ChainReactor;
  template<typename T> class ConstantReactor;
  template<typename ProducerReactorType, typename EvaluationReactorType,
    typename LeftChildReactorType, typename RightChildReactorType>
    class FoldReactor;
  template<typename T> struct FunctionEvaluation;
  template<typename FunctionType, typename... ParameterTypes>
    class FunctionReactor;
  class LuaReactorParameter;
  template<typename FunctionType> class MultiReactor;
  template<typename T> class NativeLuaReactorParameter;
  template<typename T> class NoneReactor;
  template<typename T> class ProxyReactor;
  template<typename T> class QueueReactor;
  template<typename T> class Reactor;
  class ReactorError;
  class ReactorException;
  class ReactorMonitor;
  class ReactorUnavailableException;
  template<typename ProducerReactorType> class SwitchReactor;
  template<typename T> class ThrowReactor;
  class Trigger;
  class UpdateReactor;
}
}

#endif
