#ifndef BEAM_DATA_SHUTTLE_HPP
#define BEAM_DATA_SHUTTLE_HPP
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept satisfied by types that implement the DataShuttle interface. */
  template<typename T>
  concept IsShuttle = requires(T a) {
    { a.shuttle(std::declval<const char*>(), std::declval<int&>()) } ->
      std::same_as<void>;
    { a.shuttle(std::declval<int&>()) } -> std::same_as<void>;
    { a.start_structure(std::declval<const char*>()) } -> std::same_as<void>;
    { a.end_structure() } -> std::same_as<void>;
    { a.start_sequence(std::declval<const char*>(), std::declval<int&>()) } ->
      std::same_as<void>;
    { a.start_sequence(std::declval<const char*>()) } -> std::same_as<void>;
    { a.end_sequence() } -> std::same_as<void>;
  };

  /** Concept satisfied by types that implement the Receiver interface. */
  template<typename T>
  concept IsReceiver = requires(T a) {
    typename T::Source;
    requires IsConstBuffer<typename T::Source>;
    { a.set(std::declval<Ref<const typename T::Source>>()) } ->
      std::same_as<void>;
    { a.receive(std::declval<int&>()) } -> std::same_as<void>;
    { a.receive(std::declval<const char*>(), std::declval<int&>()) } ->
      std::same_as<void>;
  } && IsShuttle<T>;

  /** Concept satisfied by types that implement the Sender interface. */
  template<typename T>
  concept IsSender = requires(T a) {
    typename T::Sink;
    requires IsBuffer<typename T::Sink>;
    { a.set(std::declval<Ref<typename T::Sink>>()) } -> std::same_as<void>;
    { a.send(std::declval<const int&>()) } -> std::same_as<void>;
    { a.send_version(std::declval<const int&>(),
      std::declval<unsigned int>()) } -> std::same_as<void>;
    { a.send(std::declval<const char*>(), std::declval<const int&>()) } ->
      std::same_as<void>;
  } && IsShuttle<T>;

  /**
   * A customization point for default constructing types.
   * @tparam T The type to default construct.
   * @return A default constructed instance of <code>T</code>.
   */
  template<typename T>
  T default_construct() = delete;

  /** Specifies a class that's responsible for shuttling data. */
  class DataShuttle {
    public:

      /** Indicates whether a type is default constructible. */
      template<typename T>
      static constexpr auto is_default_constructible =
        requires {{ default_construct<T>() } -> std::same_as<T>; } ||
        requires { T(); };

      /**
       * Shuttles an unnamed value.
       * @param value The value to shuttle.
       */
      template<typename T>
      void shuttle(T& value);

      /**
       * Shuttles a generic type.
       * @param name The name of the value.
       * @param value The value to shuttle.
       */
      template<typename T>
      void shuttle(const char* name, T& value);

      /**
       * Marks the start of a structured datum.
       * @param name The name of the structured datum.
       */
      void start_structure(const char* name);

      /** Marks the end of a structured datum. */
      void end_structure();

      /**
       * Marks the start of a sequence with a predetermined size.
       * @param name The name of the sequence.
       * @param size The number of elements in the sequence.
       */
      void start_sequence(const char* name, int& size);

      /**
       * Marks the start of a sequence with a predetermined size.
       * @param name The name of the sequence.
       * @param size The number of elements in the sequence.
       */
      void start_sequence(const char* name, const int& size);

      /**
       * Marks the start of a sequence.
       * @param name The name of the sequence.
       */
      void start_sequence(const char* name);

      /** Marks the end of a sequence. */
      void end_sequence();

    private:
      template<IsBuffer> friend struct Sender;
      template<IsConstBuffer> friend struct Receiver;
      template<typename> friend class TypeRegistry;
      template<typename, typename> friend struct Send;
      template<typename, typename> friend struct Receive;
      template<typename, typename> friend struct Shuttle;
      template<typename> friend class ReceiverMixin;
      template<typename> friend class SerializedValue;
      template<typename T>
      static T make();
      template<typename T>
      static T* make_new();
      template<IsSender S, typename T>
      static void send(S& sender, const T& value, unsigned int version);
      template<IsSender S, typename T>
      static void send(S& sender, const char* name, const T& value);
      template<IsReceiver R, typename T>
      static void receive(R& receiver, T& value, unsigned int version);
      template<IsReceiver R, typename T>
      static void receive(R& receiver, const char* name, T& value);
      template<IsShuttle S, typename T>
      static void shuttle(S& shuttle, T& value, unsigned int version);
      template<IsShuttle S, typename T>
      static void shuttle(S& shuttle, const T& value, unsigned int version);
  };

  /** Stores a type's serialization version. */
  template<typename T>
  constexpr auto shuttle_version = static_cast<unsigned int>(0);

  /**
   * Type trait for whether a type is shuttled as a structure.
   * @tparam T The type to check.
   */
  template<typename T, typename = void>
  constexpr auto is_structure = std::is_class_v<T>;

  /**
   * Type trait for whether a type is shuttled as a sequence.
   * @tparam T The type to check.
   */
  template<typename T>
  constexpr auto is_sequence = false;

  /**
   * Contains operations for shuttling a type.
   * @tparam T The type being specialized.
   */
  template<typename T, typename = void>
  struct Shuttle {

    /**
     * Shuttles a value.
     * @tparam S The type of DataShuttle to use.
     * @param shuttle The DataShuttle to use.
     * @param value The value to shuttle.
     * @param version The class version being serialized.
     */
    template<IsShuttle S>
    void operator ()(S& shuttle, T& value, unsigned int version) const;
  };

  /**
   * Shuttles a value and checks that it does not evaluate to
   * <code>nullptr</code>.
   * @tparam S The type of DataShuttle to use.
   * @tparam T The type of value to shuttle.
   * @param shuttle The DataShuttle to use.
   * @param name The name of the value being shuttled.
   * @param value The value to shuttle.
   */
  template<IsShuttle S, typename T>
  void shuttle_non_null(S& shuttle, const char* name, T& value) {
    shuttle.shuttle(name, value);
    if constexpr(IsReceiver<S>) {
      if(!value) {
        boost::throw_with_location(
          SerializationException("Invalid null value."));
      }
    }
  }

  template<typename T>
  T DataShuttle::make() {
    if constexpr(requires { { default_construct<T>() } -> std::same_as<T>; }) {
      return T(default_construct<T>());
    } else if constexpr(requires { T(); }) {
      return T();
    } else {
      static_assert(std::is_default_constructible_v<T>,
        "DataShuttle requires default-constructible T or a default_construct<T>"
        " customization.");
    }
  }

  template<typename T>
  T* DataShuttle::make_new() {
    if constexpr(requires { { default_construct<T>() } -> std::same_as<T>; }) {
      return new T(default_construct<T>());
    } else if constexpr(requires { T(); }) {
      return new T();
    }
  }

  template<IsSender S, typename T>
  void DataShuttle::send(S& sender, const T& value, unsigned int version) {
    if constexpr(requires(S& s, const T& t, unsigned int v) { t.send(s, v); }) {
      value.send(sender, version);
    } else {
      Shuttle<T>()(sender, const_cast<T&>(value), version);
    }
  }

  template<IsSender S, typename T>
  void DataShuttle::send(S& sender, const char* name, const T& value) {
    value.send(sender, name);
  }

  template<IsReceiver R, typename T>
  void DataShuttle::receive(R& receiver, T& value, unsigned int version) {
    if constexpr(requires(R& r, T& t, unsigned int v) { t.receive(r, v); }) {
      value.receive(receiver, version);
    } else {
      Shuttle<T>()(receiver, value, version);
    }
  }

  template<IsReceiver R, typename T>
  void DataShuttle::receive(R& receiver, const char* name, T& value) {
    value.receive(receiver, name);
  }

  template<IsShuttle S, typename T>
  void DataShuttle::shuttle(S& shuttle, T& value, unsigned int version) {
    value.shuttle(shuttle, version);
  }

  template<IsShuttle S, typename T>
  void DataShuttle::shuttle(S& shuttle, const T& value, unsigned int version) {
    const_cast<T&>(value).shuttle(shuttle, version);
  }

  template<typename T, typename Enabled>
  template<IsShuttle S>
  void Shuttle<T, Enabled>::operator ()(
      S& shuttle, T& value, unsigned int version) const {
    DataShuttle::shuttle(shuttle, value, version);
  }
}

#endif
