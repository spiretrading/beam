#ifndef BEAM_DATASHUTTLETESTER_HPP
#define BEAM_DATASHUTTLETESTER_HPP
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Serialization/ShuttleArray.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {

  /*! \class DataShuttleTester
      \brief Base class for testing a Serializer/Deserializer pair.
   */
  template<typename SenderType, typename ReceiverType>
  class DataShuttleTester : public CPPUNIT_NS::TestFixture {
    public:

      //! Tests shuttling a bool.
      void TestShuttlingBool();

      //! Tests shuttling a char.
      void TestShuttlingChar();

      //! Tests shuttling an unsigned char.
      void TestShuttlingUnsignedChar();

      //! Tests shuttling a short.
      void TestShuttlingShort();

      //! Tests shuttling an unsigned short.
      void TestShuttlingUnsignedShort();

      //! Tests shuttling an int.
      void TestShuttlingInt();

      //! Tests shuttling an unsigned int.
      void TestShuttlingUnsignedInt();

      //! Tests shuttling a long.
      void TestShuttlingLong();

      //! Tests shuttling an unsigned long.
      void TestShuttlingUnsignedLong();

      //! Tests shuttling a long long.
      void TestShuttlingLongLong();

      //! Tests shuttling an unsigned long long.
      void TestShuttlingUnsignedLongLong();

      //! Tests shuttling a float.
      void TestShuttlingFloat();

      //! Tests shuttling a double.
      void TestShuttlingDouble();

      //! Tests shuttling a string.
      void TestShuttlingString();

      //! Tests shuttling a sequence.
      void TestShuttlingSequence();

      //! Tests shuttling a class with a Send/Receive free functions.
      void TestShuttlingStructWithFreeShuttle();

      //! Tests shuttling a class with a Shuttle method.
      void TestShuttlingClassShuttleMethod();

      //! Tests shuttling a class with a Send and Receive methods.
      void TestShuttlingClassSendReceiveMethods();

      //! Tests shuttling a class with versioning.
      void TestShuttlingClassVersioning();

      //! Tests shuttling a nullptr.
      void TestShuttlingNullPointer();

      //! Tests shuttling a polymorphic class.
      void TestShuttlingPolymorphicClass();

      //! Tests shuttling through proxy functions.
      void TestShuttlingProxyFunctions();

      //! Tests shuttling through proxy methods.
      void TestShuttlingProxyMethods();

    protected:

      //! Makes a Sender.
      virtual SenderType MakeSender() = 0;

      //! Makes a Sender using a TypeRegistry.
      /*!
        \param registry The TypeRegistry to use.
      */
      virtual SenderType MakeSender(
        Ref<TypeRegistry<SenderType>> registry) = 0;

      //! Makes a Receiver.
      virtual ReceiverType MakeReceiver() = 0;

      //! Makes a Receiver using a TypeRegistry.
      /*!
        \param registry The TypeRegistry to use.
      */
      virtual ReceiverType MakeReceiver(
        Ref<TypeRegistry<SenderType>> registry) = 0;

    private:
      CPPUNIT_TEST_SUITE(DataShuttleTester);
        CPPUNIT_TEST(TestShuttlingBool);
        CPPUNIT_TEST(TestShuttlingChar);
        CPPUNIT_TEST(TestShuttlingUnsignedChar);
        CPPUNIT_TEST(TestShuttlingShort);
        CPPUNIT_TEST(TestShuttlingUnsignedShort);
        CPPUNIT_TEST(TestShuttlingInt);
        CPPUNIT_TEST(TestShuttlingUnsignedInt);
        CPPUNIT_TEST(TestShuttlingLong);
        CPPUNIT_TEST(TestShuttlingUnsignedLong);
        CPPUNIT_TEST(TestShuttlingLongLong);
        CPPUNIT_TEST(TestShuttlingUnsignedLongLong);
        CPPUNIT_TEST(TestShuttlingFloat);
        CPPUNIT_TEST(TestShuttlingDouble);
        CPPUNIT_TEST(TestShuttlingString);
        CPPUNIT_TEST(TestShuttlingSequence);
        CPPUNIT_TEST(TestShuttlingStructWithFreeShuttle);
        CPPUNIT_TEST(TestShuttlingClassShuttleMethod);
        CPPUNIT_TEST(TestShuttlingClassSendReceiveMethods);
        CPPUNIT_TEST(TestShuttlingClassVersioning);
        CPPUNIT_TEST(TestShuttlingNullPointer);
        CPPUNIT_TEST(TestShuttlingPolymorphicClass);
        CPPUNIT_TEST(TestShuttlingProxyFunctions);
        CPPUNIT_TEST(TestShuttlingProxyMethods);
      BEAM_CPPUNIT_TEST_SUITE_END_ABSTRACT();
  };

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingBool() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), true);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), true);
    TestShuttlingReference(MakeSender(), MakeReceiver(), false);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), false);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingChar() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), '\0');
    TestShuttlingConstant(MakeSender(), MakeReceiver(), '\0');
    TestShuttlingReference(MakeSender(), MakeReceiver(), 'a');
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 'a');
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      static_cast<signed char>(-1));
    TestShuttlingConstant(MakeSender(), MakeReceiver(),
      static_cast<signed char>(-1));
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingUnsignedChar() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), '\0');
    TestShuttlingConstant(MakeSender(), MakeReceiver(), '\0');
    TestShuttlingReference(MakeSender(), MakeReceiver(), 'a');
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 'a');
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingShort() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), static_cast<short>(0));
    TestShuttlingConstant(MakeSender(), MakeReceiver(), static_cast<short>(0));
    TestShuttlingReference(MakeSender(), MakeReceiver(), static_cast<short>(1));
    TestShuttlingConstant(MakeSender(), MakeReceiver(), static_cast<short>(1));
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      static_cast<short>(-1));
    TestShuttlingConstant(MakeSender(), MakeReceiver(), static_cast<short>(-1));
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingUnsignedShort() {
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      static_cast<unsigned short>(0));
    TestShuttlingConstant(MakeSender(), MakeReceiver(),
      static_cast<unsigned short>(0));
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      static_cast<unsigned short>(1));
    TestShuttlingConstant(MakeSender(), MakeReceiver(),
      static_cast<unsigned short>(1));
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingInt() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1);
    TestShuttlingReference(MakeSender(), MakeReceiver(), -1);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), -1);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingUnsignedInt() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0U);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0U);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1U);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1U);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingLong() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0L);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0L);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1L);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1L);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingUnsignedLong() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0UL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0UL);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1UL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1UL);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingLongLong() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0LL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0LL);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1LL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1LL);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingUnsignedLongLong() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0ULL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0ULL);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1ULL);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1ULL);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingFloat() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0.0F);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0.0F);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1.0F);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1.0F);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingDouble() {
    TestShuttlingReference(MakeSender(), MakeReceiver(), 0.0);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 0.0);
    TestShuttlingReference(MakeSender(), MakeReceiver(), 1.0);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), 1.0);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingString() {
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      std::string("hello world"));
    TestShuttlingReference(MakeSender(), MakeReceiver(), std::string(""));
    TestShuttlingConstant(MakeSender(), MakeReceiver(),
      std::string("hello world"));
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingSequence() {
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      std::array<int, 5>{5, 4, 3, 2, 1});
    TestShuttlingReference(MakeSender(), MakeReceiver(),
      std::array<int, 3>{1, 2, 3});
    TestShuttlingConstant(MakeSender(), MakeReceiver(),
      std::array<int, 4>{4, 1, 2, 3});
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingStructWithFreeShuttle() {
    StructWithFreeShuttle object = {'1', 2, 3.3};
    TestShuttlingReference(MakeSender(), MakeReceiver(), object);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), object);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingClassShuttleMethod() {
    ClassWithShuttleMethod object('1', 2, 3.3);
    TestShuttlingReference(MakeSender(), MakeReceiver(), object);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), object);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingClassSendReceiveMethods() {
    ClassWithSendReceiveMethods object('1', 2, 3.3);
    TestShuttlingReference(MakeSender(), MakeReceiver(), object);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), object);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingClassVersioning() {
    ClassWithVersioning outValue(1, 2, 3);
    auto sender = MakeSender();
    typename SenderType::Sink buffer;
    ClassWithVersioning inValue;
    auto receiver = MakeReceiver();

    // Shuttle version 0.
    sender.SetSink(Ref(buffer));
    sender.Send(outValue, 0);
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(inValue);
    CPPUNIT_ASSERT(inValue == ClassWithVersioning(1, 0, 0));
    buffer.Reset();

    // Shuttle version 1.
    sender.SetSink(Ref(buffer));
    sender.Send(outValue, 1);
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(inValue);
    CPPUNIT_ASSERT(inValue == ClassWithVersioning(1, 2, 0));
    buffer.Reset();

    // Shuttle version 2.
    sender.SetSink(Ref(buffer));
    sender.Send(outValue, 2);
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(inValue);
    CPPUNIT_ASSERT(inValue == ClassWithVersioning(1, 2, 3));
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::TestShuttlingNullPointer() {
    PolymorphicDerivedClassA* outValue = nullptr;
    TypeRegistry<SenderType> typeRegistry;
    typeRegistry.template Register<PolymorphicDerivedClassA>(
      "PolymorphicDerivedClassA");
    auto sender = MakeSender(Ref(typeRegistry));
    auto receiver = MakeReceiver(Ref(typeRegistry));
    typename SenderType::Sink buffer;
    sender.SetSink(Ref(buffer));
    sender.Send(outValue);
    PolymorphicBaseClass* inValue;
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(inValue);
    CPPUNIT_ASSERT(inValue == nullptr);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingPolymorphicClass() {
    std::unique_ptr<PolymorphicDerivedClassA> outValue(
      new PolymorphicDerivedClassA());
    TypeRegistry<SenderType> typeRegistry;
    typeRegistry.template Register<PolymorphicDerivedClassA>(
      "PolymorphicDerivedClassA");
    typeRegistry.template Register<PolymorphicDerivedClassB>(
      "PolymorphicDerivedClassB");
    auto sender = MakeSender(Ref(typeRegistry));
    auto receiver = MakeReceiver(Ref(typeRegistry));
    typename SenderType::Sink buffer;
    sender.SetSink(Ref(buffer));
    sender.Send(outValue.get());
    PolymorphicBaseClass* inValue;
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(inValue);
    CPPUNIT_ASSERT(inValue->ToString() == outValue->ToString());
    delete inValue;
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingProxyFunctions() {
    ProxiedFunctionType object("hello world");
    TestShuttlingReference(MakeSender(), MakeReceiver(), object);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), object);
  }

  template<typename SenderType, typename ReceiverType>
  void DataShuttleTester<SenderType, ReceiverType>::
      TestShuttlingProxyMethods() {
    ProxiedMethodType object("hello world");
    TestShuttlingReference(MakeSender(), MakeReceiver(), object);
    TestShuttlingConstant(MakeSender(), MakeReceiver(), object);
  }
}
}
}

#endif
