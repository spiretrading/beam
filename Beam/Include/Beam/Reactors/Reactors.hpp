#ifndef BEAM_REACTORS_HPP
#define BEAM_REACTORS_HPP

namespace Beam {
namespace Reactors {
  class BaseReactor;
  template<typename T> class ConstantReactor;
  template<typename T> class NoneReactor;
  template<typename T> class Reactor;
  class ReactorError;
  class ReactorException;
  class ReactorUnavailableException;
  class Trigger;
}
}

#endif
