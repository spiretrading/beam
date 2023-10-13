#ifndef BEAM_SHUTTLE_TEST_TYPES_HPP
#define BEAM_SHUTTLE_TEST_TYPES_HPP
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/SerializationTests/SerializationTests.hpp"

namespace Beam::Serialization {
namespace Tests {

  /** Struct with a free symmetric shuttle. */
  struct StructWithFreeShuttle {
    char m_a;
    int m_b;
    double m_c;

    bool operator ==(const StructWithFreeShuttle& rhs) const = default;
  };

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

      bool operator ==(const ClassWithShuttleMethod& rhs) const = default;

    private:
      friend struct Serialization::DataShuttle;
      char m_a;
      int m_b;
      double m_c;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version) {
        shuttle.Shuttle("a", m_a);
        shuttle.Shuttle("b", m_b);
        shuttle.Shuttle("c", m_c);
      }
  };

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

      bool operator ==(const ClassWithSendReceiveMethods& rhs) const = default;

    private:
      friend struct Serialization::DataShuttle;
      char m_a;
      int m_b;
      double m_c;

      template<typename Shuttler>
      void Send(Shuttler& shuttle, unsigned int version) const {
        shuttle.Shuttle("a", m_a);
        auto b1 = m_b - 1;
        shuttle.Shuttle("b1", b1);
        auto b2 = m_b + 1;
        shuttle.Shuttle("b2", b2);
        shuttle.Shuttle("c", m_c);
      }

      template<typename Shuttler>
      void Receive(Shuttler& shuttle, unsigned int version) {
        shuttle.Shuttle("a", m_a);
        auto b1 = int();
        shuttle.Shuttle("b1", b1);
        auto b2 = int();
        shuttle.Shuttle("b2", b2);
        m_b = b1 + 1;
        REQUIRE(b2 == m_b + 1);
        shuttle.Shuttle("c", m_c);
      }
  };

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

      bool operator ==(const ClassWithVersioning& rhs) const = default;

    private:
      friend struct Serialization::DataShuttle;
      int m_v0;
      int m_v1;
      int m_v2;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version) {
        shuttle.Shuttle("v0", m_v0);
        if(version >= 1) {
          shuttle.Shuttle("v1", m_v1);
        } else if(Serialization::IsReceiver<Shuttler>::value) {
          m_v1 = 0;
        }
        if(version >= 2) {
          shuttle.Shuttle("v2", m_v2);
        } else if(Serialization::IsReceiver<Shuttler>::value) {
          m_v2 = 0;
        }
      }
  };

  /** Base class of a polymorphic type. */
  class PolymorphicBaseClass {
    public:
      virtual ~PolymorphicBaseClass() = default;

      /** Returns an identifier for this class. */
      virtual std::string ToString() const = 0;
  };

  /** Inherits PolymorphicBaseClass. */
  class PolymorphicDerivedClassA : public PolymorphicBaseClass {
    public:

      /** Constructs a PolymorphicDerivedClassA. */
      PolymorphicDerivedClassA() = default;

      virtual std::string ToString() const override;

    private:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version) {}
  };

  /** Inherits PolymorphicBaseClass. */
  class PolymorphicDerivedClassB : public PolymorphicBaseClass {
    public:

      /** Constructs a PolymorphicDerivedClassB. */
      PolymorphicDerivedClassB() = default;

      virtual std::string ToString() const override;

    private:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version) {}
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
      std::string ToString() const;

      bool operator ==(const ProxiedFunctionType& rhs) const = default;

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
      std::string ToString() const;

      bool operator ==(const ProxiedMethodType& rhs) const = default;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_value;

      template<typename Shuttler>
      void Send(Shuttler& shuttle, const char* name) const {
        shuttle.Send(name, m_value);
      }

      template<typename Shuttler>
      void Receive(Shuttler& shuttle, const char* name) {
        shuttle.Shuttle(name, m_value);
      }
  };
}

  template<>
  struct Shuttle<Tests::StructWithFreeShuttle> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Tests::StructWithFreeShuttle& value,
        unsigned int version) const {
      shuttle.Shuttle("a", value.m_a);
      shuttle.Shuttle("b", value.m_b);
      shuttle.Shuttle("c", value.m_c);
    }
  };

  template<>
  struct Version<Tests::ClassWithVersioning> :
    std::integral_constant<unsigned int, 2> {};

  template<>
  struct IsStructure<Tests::ProxiedFunctionType> : std::false_type {};

  template<>
  struct IsStructure<Tests::ProxiedMethodType> : std::false_type {};

  template<>
  struct Send<Tests::ProxiedFunctionType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Tests::ProxiedFunctionType& value) const {
      shuttle.Send(name, value.ToString());
    }
  };

  template<>
  struct Receive<Tests::ProxiedFunctionType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Tests::ProxiedFunctionType& value) const {
      auto proxy = std::string();
      shuttle.Shuttle(name, proxy);
      value = Tests::ProxiedFunctionType(proxy);
    }
  };
}

#endif
