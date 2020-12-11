#ifndef BEAM_SERIALIZATION_HPP
#define BEAM_SERIALIZATION_HPP

namespace Beam::Serialization {
  template<typename S> class BinaryReceiver;
  template<typename S> class BinarySender;
  struct DataShuttle;
  template<typename T> struct Inverse;
  template<typename T, typename Enabled = void> struct IsReceiver;
  template<typename T, typename Enabled = void> struct IsSender;
  template<typename T> struct IsSequence;
  template<typename T, typename Enabled> struct IsStructure;
  template<typename S> class JsonReceiver;
  template<typename S> class JsonSender;
  template<typename S> struct Receiver;
  template<typename R> class ReceiverMixin;
  template<typename S> struct Sender;
  template<typename S> class SenderMixin;
  class SerializationException;
  template<typename T> class SerializedValue;
  template<typename SenderType> class TypeEntry;
  template<typename SenderType> class TypeRegistry;
  class TypeNotFoundException;
  template<typename T> struct Version;
}

#endif
