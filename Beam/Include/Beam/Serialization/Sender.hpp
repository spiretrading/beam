#ifndef BEAM_SENDER_HPP
#define BEAM_SENDER_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace Serialization {
namespace Details {
  BEAM_DEFINE_HAS_METHOD(SenderHasSendMethod, Send, void, const char*,
    unsigned int version);
  BEAM_DEFINE_HAS_TYPEDEF(SenderHasSinkType, Sink);
}

  /**
   * Specifies a DataShuttle that's responsible for sending data.
   * @param <S> The type of Buffer used to store the data.
   */
  template<typename S>
  struct Sender : Concept<DataShuttle>, Concept<Sender<S>> {

    /** The type of Buffer used to store the data. */
    using Sink = S;

    /**
     * Specifies where the data to be sent is stored.
     * @param sink Where the data to send is stored.
     */
    void SetSink(Ref<Sink> sink);

    /**
     * Sends a value.
     * @param value The value to send.
     */
    template<typename T>
    void Send(const T& value);

    /**
     * Sends a value, used to explicitly specify a value's version.
     * @param value The value to send.
     * @param version The version of the value to send.
     */
    template<typename T>
    void Send(const T& value, unsigned int version);

    /**
     * Sends a value, used to explicitly specify a value's version.
     * @param name The name of the value to send.
     * @param value The value to send.
     * @param version The version of the value to send.
     */
    template<typename T>
    void Send(const char* name, const T& value, unsigned int version);

    private:
      template<typename T, typename Enabled> friend struct Send;
      template<bool HasMethod, typename Dummy>
      struct ResolveDirectMethods {
        template<typename Shuttler, typename T>
        void operator ()(Shuttler& shuttle, const T& value,
          unsigned int version);
      };
      template<typename Dummy>
      struct ResolveDirectMethods<false, Dummy> {
        template<typename Shuttler, typename T>
        void operator ()(Shuttler& shuttle, const T& value,
          unsigned int version);
      };
  };

  /**
   * Checks if a specified type is a Sender.
   * @param <T> The type to check.
   */
  template<typename T, typename Enabled>
  struct IsSender : std::false_type {};

  /**
   * Contains operations for sending a type.
   * @param <T> The type being specialized.
   */
  template<typename T, typename Enabled = void>
  struct Send {

    /**
     * Sends a value.
     * @param <Shuttler> The type of Sender to use.
     * @param shuttle The Sender to use.
     * @param value The value to send.
     * @param version The class version being serialized.
     */
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const T& value,
      unsigned int version) const;

    /**
     * Sends a value.
     * @tparam <Shuttler> The type of Sender to use.
     * @param shuttle The Sender to use.
     * @param name The name of the value to send.
     * @param value The value to send.
     */
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name, const T& value) const;
  };

  template<typename T>
  struct IsSender<T, std::enable_if_t<Details::SenderHasSinkType<T>::value>> :
    ImplementsConcept<T, Sender<typename T::Sink>>::type {};

  template<typename S>
  template<bool HasMethod, typename Dummy>
  template<typename Shuttler, typename T>
  void Sender<S>::ResolveDirectMethods<HasMethod, Dummy>::operator ()(
      Shuttler& shuttle, const T& value, unsigned int version) {
    DataShuttle::Send(shuttle, value, version);
  }

  template<typename S>
  template<typename Dummy>
  template<typename Shuttler, typename T>
  void Sender<S>::ResolveDirectMethods<false, Dummy>::operator ()(
      Shuttler& shuttle, const T& value, unsigned int version) {
    Shuttle<T>()(shuttle, const_cast<T&>(value), version);
  }

  template<typename T, typename Enabled>
  template<typename Shuttler>
  void Send<T, Enabled>::operator ()(Shuttler& shuttle, const T& value,
      unsigned int version) const {
    typename Sender<typename Shuttler::Sink>::template ResolveDirectMethods<
      Details::SenderHasSendMethod<T, Shuttler>::value, void>()(shuttle, value,
        version);
  }

  template<typename T, typename Enabled>
  template<typename Shuttler>
  void Send<T, Enabled>::operator ()(Shuttler& shuttle, const char* name,
      const T& value) const {
    DataShuttle::Send(shuttle, name, value);
  }

  template<typename Buffer, typename Shuttler, typename T>
  Buffer Encode(Shuttler& shuttler, const T& data) {
    auto buffer = Buffer();
    shuttler.SetSink(Ref(buffer));
    shuttler.Send(data);
    return buffer;
  }
}

  template<typename T>
  struct ImplementsConcept<T, std::enable_if_t<
    Serialization::IsSender<T>::value, Serialization::DataShuttle>> :
    std::true_type {};
}

#endif
