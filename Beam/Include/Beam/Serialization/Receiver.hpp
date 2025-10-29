#ifndef BEAM_RECEIVER_HPP
#define BEAM_RECEIVER_HPP
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializedValue.hpp"

namespace Beam {

  /**
   * Specifies a DataShuttle that's responsible for receiving data.
   * @tparam S A type of Buffer used to store the data.
   */
  template<IsConstBuffer S>
  class Receiver {
    public:

      /** The type of Buffer used to store the data. */
      using Source = S;

      /**
       * Specifies where the received data is stored.
       * @param source Where the received data is stored.
       */
      void set(Ref<const Source> source);

      /**
       * Receives a value.
       * @param value The value to receive.
       */
      template<typename T>
      void receive(T& value);

      /**
       * Receives a value.
       * @param name The name of the value to receive.
       * @param value The value to receive.
       */
      template<typename T>
      void receive(const char* name, T& value);
  };

  /**
   * Contains operations for receiving a type.
   * @tparam T The type being specialized.
   */
  template<typename T, typename = void>
  struct Receive {

    /**
     * Receives a value.
     * @param receiver The Receiver to use.
     * @param value The value to receive.
     * @param version The class version being serialized.
     */
    template<IsReceiver R>
    void operator ()(R& receiver, T& value, unsigned int version) const;

    /**
     * Receives a value.
     * @param receiver The Receiver to use.
     * @param name The name of the value to receive.
     * @param value The value to receive.
     */
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, T& value) const;
  };

  /**
   * Returns a value received by a Receiver.
   * @tparam T The type of value to receive.
   * @param receiver The Receiver to use.
   * @param name The name of the value to receive.
   * @return The value received by the <i>receiver</i>.
   */
  template<typename T, IsReceiver R>
  T receive(R& receiver, const char* name) {
    auto value = SerializedValue<T>();
    value.initialize();
    receiver.receive(name, *value);
    return std::move(*value);
  }

  /**
   * Returns a value received by a Receiver.
   * @tparam T The type of value to receive.
   * @param receiver The Receiver to use.
   * @return The value received by the <i>receiver</i>.
   */
  template<typename T, IsReceiver R>
  T receive(R& receiver) {
    auto value = SerializedValue<T>();
    value.initialize();
    receiver.receive(*value);
    return std::move(*value);
  }

  template<typename T, typename Enabled>
  template<IsReceiver R>
  void Receive<T, Enabled>::operator ()(
      R& receiver, T& value, unsigned int version) const {
    DataShuttle::receive(receiver, value, version);
  }

  template<typename T, typename Enabled>
  template<IsReceiver R>
  void Receive<T, Enabled>::operator ()(
      R& receiver, const char* name, T& value) const {
    DataShuttle::receive(receiver, name, value);
  }
}

#endif
