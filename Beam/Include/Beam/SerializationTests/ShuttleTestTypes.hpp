#ifndef BEAM_SHUTTLE_TEST_TYPES_HPP
#define BEAM_SHUTTLE_TEST_TYPES_HPP
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Tests {

  /** Struct with a free symmetric shuttle. */
  struct StructWithFreeShuttle {
    char m_a;
    int m_b;
    double m_c;

    bool operator ==(const StructWithFreeShuttle&) const = default;
  };

  std::ostream& operator <<(
    std::ostream& out, const StructWithFreeShuttle& value);

  /** Class type with a symmetric shuttle method. */
  class ClassWithShuttleMethod {
    public:

      /** Constructs an uninitialized ClassWithShuttleMethod. */
      ClassWithShuttleMethod() = default;

      /**
       * Constructs a ClassWithShuttleMethod.
       * @param a The char value to shuttle.
       * @param b The int value to shuttle.
       * @param c The double value to shuttle.
       */
      ClassWithShuttleMethod(char a, int b, double c);

      bool operator ==(const ClassWithShuttleMethod&) const = default;

    private:
      friend std::ostream& operator <<(
        std::ostream& out, const ClassWithShuttleMethod& value);
      friend struct Beam::DataShuttle;
      char m_a;
      int m_b;
      double m_c;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version) {
        shuttle.shuttle("a", m_a);
        shuttle.shuttle("b", m_b);
        shuttle.shuttle("c", m_c);
      }
  };

  std::ostream& operator <<(
    std::ostream& out, const ClassWithShuttleMethod& value);

  /** Class type with different methods for sending/receiving. */
  class ClassWithSendReceiveMethods {
    public:

      /** Constructs an uninitialized ClassWithSendReceiveMethods. */
      ClassWithSendReceiveMethods() = default;

      /**
       * Constructs a ClassWithSendReceiveMethods.
       * @param a The char value to shuttle.
       * @param b The int value to shuttle.
       * @param c The double value to shuttle.
       */
      ClassWithSendReceiveMethods(char a, int b, double c);

      bool operator ==(const ClassWithSendReceiveMethods&) const = default;

    private:
      friend std::ostream& operator <<(
        std::ostream& out, const ClassWithSendReceiveMethods& value);
      friend struct Beam::DataShuttle;
      char m_a;
      int m_b;
      double m_c;

      template<IsSender S>
      void send(S& sender, unsigned int version) const {
        sender.shuttle("a", m_a);
        auto b1 = m_b - 1;
        sender.shuttle("b1", b1);
        auto b2 = m_b + 1;
        sender.shuttle("b2", b2);
        sender.shuttle("c", m_c);
      }

      template<IsReceiver R>
      void receive(R& receiver, unsigned int version) {
        receiver.shuttle("a", m_a);
        auto b1 = int();
        receiver.shuttle("b1", b1);
        auto b2 = int();
        receiver.shuttle("b2", b2);
        m_b = b1 + 1;
        REQUIRE(b2 == m_b + 1);
        receiver.shuttle("c", m_c);
      }
  };

  std::ostream& operator <<(
    std::ostream& out, const ClassWithSendReceiveMethods& value);

  /** Class type with versioning. */
  class ClassWithVersioning {
    public:

      /** Constructs an uninitialized ClassWithVersioning. */
      ClassWithVersioning() = default;

      /**
       * Constructs a ClassWithVersioning.
       * @param v0 Value visible in version 0.
       * @param v1 Value visible in version 1.
       * @param v2 Value visible in version 2.
       */
      ClassWithVersioning(int v0, int v1, int v2);

      bool operator ==(const ClassWithVersioning&) const = default;

    private:
      friend std::ostream& operator <<(
        std::ostream& out, const ClassWithVersioning& value);
      friend struct Beam::DataShuttle;
      int m_v0;
      int m_v1;
      int m_v2;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version) {
        shuttle.shuttle("v0", m_v0);
        if(version >= 1) {
          shuttle.shuttle("v1", m_v1);
        } else if(IsReceiver<S>) {
          m_v1 = 0;
        }
        if(version >= 2) {
          shuttle.shuttle("v2", m_v2);
        } else if(IsReceiver<S>) {
          m_v2 = 0;
        }
      }
  };

  std::ostream& operator <<(
    std::ostream& out, const ClassWithVersioning& value);

  /** Base class of a polymorphic type. */
  class PolymorphicBaseClass {
    public:
      virtual ~PolymorphicBaseClass() = default;

      /** Returns an identifier for this class. */
      virtual std::string to_string() const = 0;
  };

  /** Inherits PolymorphicBaseClass. */
  class PolymorphicDerivedClassA : public PolymorphicBaseClass {
    public:

      /** Constructs a PolymorphicDerivedClassA. */
      PolymorphicDerivedClassA() = default;

      std::string to_string() const override;

    private:
      friend struct Beam::DataShuttle;

      template<typename Shuttler>
      void shuttle(Shuttler& shuttle, unsigned int version) {}
  };

  /** Inherits PolymorphicBaseClass. */
  class PolymorphicDerivedClassB : public PolymorphicBaseClass {
    public:

      /** Constructs a PolymorphicDerivedClassB. */
      PolymorphicDerivedClassB() = default;

      virtual std::string to_string() const override;

    private:
      friend struct Beam::DataShuttle;

      template<typename Shuttler>
      void shuttle(Shuttler& shuttle, unsigned int version) {}
  };

  /** Tests shuttling via proxy functions. */
  class ProxiedFunctionType {
    public:

      /** Constructs an empty ProxiedFunctionType. */
      ProxiedFunctionType() = default;

      /**
       * Constructs a ProxiedFunctionType.
       * @param value The value to return in the ToString.
       */
      explicit ProxiedFunctionType(const std::string& value);

      /** Returns the held value. */
      std::string to_string() const;

      bool operator ==(const ProxiedFunctionType&) const = default;

    private:
      std::string m_value;
  };

  /** Tests shuttling via proxy methods. */
  class ProxiedMethodType {
    public:

      /** Constructs an empty ProxiedMethodType. */
      ProxiedMethodType() = default;

      /**
       * Constructs a ProxiedMethodType.
       * @param value The value to return in the ToString.
       */
      explicit ProxiedMethodType(const std::string& value);

      /** Returns the held value. */
      std::string to_string() const;

      bool operator ==(const ProxiedMethodType&) const = default;

    private:
      friend struct Beam::DataShuttle;
      std::string m_value;

      template<typename Shuttler>
      void send(Shuttler& shuttle, const char* name) const {
        shuttle.send(name, m_value);
      }

      template<typename Shuttler>
      void receive(Shuttler& shuttle, const char* name) {
        shuttle.receive(name, m_value);
      }
  };
}

  template<>
  struct Shuttle<Tests::StructWithFreeShuttle> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Tests::StructWithFreeShuttle& value,
        unsigned int version) const {
      shuttle.shuttle("a", value.m_a);
      shuttle.shuttle("b", value.m_b);
      shuttle.shuttle("c", value.m_c);
    }
  };

  template<>
  constexpr auto shuttle_version<Tests::ClassWithVersioning> = unsigned(2);

  template<>
  constexpr auto is_structure<Tests::ProxiedFunctionType> = false;

  template<>
  constexpr auto is_structure<Tests::ProxiedMethodType> = false;

  template<>
  struct Send<Tests::ProxiedFunctionType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Tests::ProxiedFunctionType& value) const {
      shuttle.send(name, value.to_string());
    }
  };

  template<>
  struct Receive<Tests::ProxiedFunctionType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Tests::ProxiedFunctionType& value) const {
      auto proxy = std::string();
      shuttle.receive(name, proxy);
      value = Tests::ProxiedFunctionType(proxy);
    }
  };
}

#endif
