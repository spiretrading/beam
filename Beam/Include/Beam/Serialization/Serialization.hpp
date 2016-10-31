#ifndef BEAM_SERIALIZATION_HPP
#define BEAM_SERIALIZATION_HPP

namespace Beam {
namespace Serialization {
  template<typename SourceType> class BinaryReceiver;
  template<typename SinkType> class BinarySender;
  struct DataShuttle;
  template<typename T> struct Inverse;
  template<typename T, typename Enabled = void> struct IsReceiver;
  template<typename T, typename Enabled = void> struct IsSender;
  template<typename T> struct IsSequence;
  template<typename T, typename Enabled> struct IsStructure;
  template<typename SourceType> class JsonReceiver;
  template<typename SinkType> class JsonSender;
  template<typename SourceType> struct Receiver;
  template<typename ReceiverType> class ReceiverMixin;
  template<typename SinkType> struct Sender;
  template<typename SenderType> class SenderMixin;
  class SerializationException;
  template<typename T> class SerializedValue;
  template<typename SenderType> class TypeEntry;
  template<typename SenderType> class TypeRegistry;
  class TypeNotFoundException;
  template<typename T> struct Version;
}
}

#endif
