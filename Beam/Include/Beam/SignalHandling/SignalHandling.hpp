#ifndef BEAM_SIGNALHANDLING_HPP
#define BEAM_SIGNALHANDLING_HPP

#define BEAM_SIGNAL_HANDLING_PARAMETER_COUNT 10

namespace Beam {
namespace SignalHandling {
  template<typename SlotType> class ActivationSlot;
  class ConnectionGroup;
  class GroupConnection;
  struct NullSlot;
  class QueuedSignalHandler;
  class ScopedGroupConnection;
  class ScopedSlotAdaptor;
  class SignalSink;
  class TaskSignalHandler;
  class TriggerSlot;
}
}

#endif
