#ifndef BEAM_REACTOR_TESTS_HPP
#define BEAM_REACTOR_TESTS_HPP
#include <stdexcept>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Reactors/Reactor.hpp"

namespace Beam {
namespace Reactors {
namespace Tests {
  class BasicReactorTester;
  class ChainReactorTester;
  class ConstantReactorTester;
  class DoReactorTester;
  class FirstReactorTester;
  class FunctionReactorTester;
  class LastReactorTester;
  class MultiReactorTester;
  class NoneReactorTester;
  class NonRepeatingReactorTester;
  class QueueReactorTester;
  class ReactorMonitorTester;
  class TimerReactorTester;
  class TriggerTester;
  class UpdateReactorTester;

  /*! \struct DummyException
      \brief Exception class used for testing purposes.
   */
  struct DummyException : std::exception {};

  //! Asserts that a Reactor evaluates to a specified value after a sequence
  //! point.
  /*!
    \param reactor The Reactor to test.
    \param sequenceNumber The sequence number the Reactor is expecting.
    \param update The type of update at the <i>sequenceNumber</i>.
    \param expectedValue The expected evaluation.
  */
  template<typename T, typename U>
  void AssertValue(Reactor<T>& reactor, int sequenceNumber,
      BaseReactor::Update update, const U& expectedValue) {
    CPPUNIT_ASSERT(reactor.Commit(sequenceNumber) == update);
    CPPUNIT_ASSERT(reactor.Commit(sequenceNumber) == update);
    CPPUNIT_ASSERT(reactor.Eval() == expectedValue);
  }

  //! Asserts that a Reactor throws an exception after a sequence point.
  /*!
    \param reactor The Reactor to test.
    \param sequenceNumber The sequence number the Reactor is expecting.
    \param update The type of update at the <i>sequenceNumber</i>.
  */
  template<typename E, typename T>
  void AssertException(Reactor<T>& reactor, int sequenceNumber,
      BaseReactor::Update update) {
    CPPUNIT_ASSERT(reactor.Commit(sequenceNumber) == update);
    try {
      reactor.Eval();
      CPPUNIT_FAIL("Expected exception not thrown.");
    } catch(const E&) {
    } catch(const std::exception&) {
      CPPUNIT_FAIL("Expected exception not thrown.");
    }
  }
}
}
}

#endif
