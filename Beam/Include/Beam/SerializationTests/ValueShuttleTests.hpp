#ifndef BEAM_VALUESHUTTLETESTS_HPP
#define BEAM_VALUESHUTTLETESTS_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/SerializationTests/SerializationTests.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {

  //! Tests shuttling a value through a reference.
  /*!
    \param sender The Sender to use.
    \param receiver The Receiver to use.
    \param value The value to shuttle.
  */
  template<typename SenderType, typename ReceiverType, typename T>
  static void TestShuttlingReference(SenderType&& sender,
      ReceiverType&& receiver, const T& value) {
    T inValue = value;
    typename SenderType::Sink buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(inValue);
    receiver.SetSource(Ref(buffer));
    T outValue;
    receiver.Shuttle(outValue);
    CPPUNIT_ASSERT(inValue == outValue);
  }

  //! Tests shuttling a value directly.
  /*!
    \param sender The Sender to use.
    \param receiver The Receiver to use.
    \param value The value to shuttle.
  */
  template<typename SenderType, typename ReceiverType, typename T>
  static void TestShuttlingConstant(SenderType&& sender,
      ReceiverType&& receiver, const T& value) {
    typename SenderType::Sink buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    receiver.SetSource(Ref(buffer));
    T outValue;
    receiver.Shuttle(outValue);
    CPPUNIT_ASSERT(value == outValue);
  }
}
}
}

#endif
