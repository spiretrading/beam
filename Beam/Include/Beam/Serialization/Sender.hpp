#ifndef BEAM_SENDER_HPP
#define BEAM_SENDER_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Specifies a DataShuttle that's responsible for sending data.
   * @tparam S The type of Buffer used to store the data.
   */
  template<IsBuffer S>
  class Sender {
    public:

      /** The type of Buffer used to store the data. */
      using Sink = S;

      /**
       * Specifies where the data to be sent is stored.
       * @param sink Where the data to send is stored.
       */
      void set(Ref<Sink> sink);

      /**
       * Sends a value.
       * @param value The value to send.
       */
      template<typename T>
      void send(const T& value);

      /**
       * Sends a value.
       * @param value The value to send.
       * @param version The class version being serialized.
       */
      template<typename T>
      void send_version(const T& value, unsigned int version);

      /**
       * Sends a value.
       * @param name The name of the value to send.
       * @param value The value to send.
       */
      template<typename T>
      void send(const char* name, const T& value);
  };

  /**
   * Contains operations for sending a type.
   * @tparam T The type being specialized.
   */
  template<typename T, typename = void>
  struct Send {

    /**
     * Sends a value.
     * @param sender The Sender to use.
     * @param value The value to send.
     * @param version The class version being serialized.
     */
    template<IsSender S>
    void operator ()(S& sender, const T& value, unsigned int version) const;

    /**
     * Sends a value.
     * @param sender The Sender to use.
     * @param name The name of the value to send.
     * @param value The value to send.
     */
    template<IsSender S>
    void operator ()(S& sender, const char* name, const T& value) const;
  };

  template<typename T, typename Enabled>
  template<IsSender S>
  void Send<T, Enabled>::operator ()(
      S& sender, const T& value, unsigned int version) const {
    DataShuttle::send(sender, value, version);
  }

  template<typename T, typename Enabled>
  template<IsSender S>
  void Send<T, Enabled>::operator ()(
      S& sender, const char* name, const T& value) const {
    DataShuttle::send(sender, name, value);
  }

  template<IsBuffer B, IsSender S, typename T>
  B encode(S& sender, const T& data) {
    auto buffer = B();
    sender.set(Ref(buffer));
    sender.send(data);
    return buffer;
  }
}

#endif
