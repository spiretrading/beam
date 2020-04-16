#ifndef BEAM_VALUE_SHUTTLE_TESTS_HPP
#define BEAM_VALUE_SHUTTLE_TESTS_HPP
#include <doctest/doctest.h>
#include "Beam/Pointers/Ref.hpp"

namespace Beam::Serialization::Tests {

  //! Tests shuttling a value through a reference.
  /*!
    \param sender The Sender to use.
    \param receiver The Receiver to use.
    \param value The value to shuttle.
  */
  template<typename SenderType, typename ReceiverType, typename T>
  void TestShuttlingReference(SenderType&& sender, ReceiverType&& receiver,
      const T& value) {
    auto inValue = value;
    auto buffer = typename SenderType::Sink();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(inValue);
    receiver.SetSource(Ref(buffer));
    auto outValue = T();
    receiver.Shuttle(outValue);
    REQUIRE(inValue == outValue);
  }

  //! Tests shuttling a value directly.
  /*!
    \param sender The Sender to use.
    \param receiver The Receiver to use.
    \param value The value to shuttle.
  */
  template<typename SenderType, typename ReceiverType, typename T>
  void TestShuttlingConstant(SenderType&& sender, ReceiverType&& receiver,
      const T& value) {
    auto buffer = typename SenderType::Sink();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(value);
    receiver.SetSource(Ref(buffer));
    auto outValue = T();
    receiver.Shuttle(outValue);
    REQUIRE(value == outValue);
  }
}

#endif
