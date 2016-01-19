#ifndef BEAM_SERIALIZATIONTESTS_HPP
#define BEAM_SERIALIZATIONTESTS_HPP
#include "Beam/Serialization/Serialization.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {
  class BinaryShuttleTester;
  class ClassWithShuttleMethod;
  class ClassWithSendReceiveMethods;
  class ClassWithVersioning;
  template<typename SenderType, typename ReceiverType> class DataShuttleTester;
  class PolymorphicBaseClass;
  class ProxiedFunctionType;
  class ProxiedMethodType;
}
}
}

#endif
