#ifndef BEAM_QUEUE_READER_HPP
#define BEAM_QUEUE_READER_HPP
#include <concepts>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/BaseQueue.hpp"

namespace Beam {

  /**
   * Interface for the read-only side of a Queue.
   * @tparam T The data to read from the Queue.
   */
  template<typename T>
  class QueueReader : public virtual BaseQueue {
    public:

      /** The type being read. */
      using Source = T;

      /**
       * Returns the first value in the queue and pops it, blocking until a
       * value is available.
       */
      virtual Source pop() = 0;

      /**
       * Returns the first value in the queue if one is available and pops it
       * without blocking, otherwise returns <i>boost::none</i>.
       */
      virtual boost::optional<Source> try_pop() = 0;
  };

  /**
   * Invokes a callable for each value popped from a queue until the queue is
   * broken.
   * @param queue The QueueReader to read values from.
   * @param f A callable to apply to each value popped of of the
   *        <code>queue</code>.
   * @param on_break A callable to invoke when the <code>queue</code> is
   *        broken.
   */
  template<typename Q, typename F1, typename F2> requires
    std::derived_from<
      dereference_t<Q>, QueueReader<typename dereference_t<Q>::Source>> &&
    std::invocable<F1, typename dereference_t<Q>::Source> &&
    std::invocable<F2, std::exception_ptr>
  void for_each(const Q& queue, F1 f, F2&& on_break) {
    try {
      while(true) {
        f(queue->pop());
      }
    } catch(const std::exception&) {
      std::forward<F2>(on_break)(std::current_exception());
    }
  }

  /**
   * Invokes a callable for each value popped from a queue until the queue is
   * broken.
   * @param queue The QueueReader to read values from.
   * @param f A callable to apply to each value popped of of the
   *        <code>queue</code>.
   */
  template<typename Q, typename F> requires
    std::derived_from<
      dereference_t<Q>, QueueReader<typename dereference_t<Q>::Source>> &&
    std::invocable<F, typename dereference_t<Q>::Source>
  void for_each(const Q& queue, F f) {
    for_each(queue, std::move(f), [] (auto&&) {});
  }

  /**
   * Flushes the contents of a QueueReader into an iterator.
   * @param queue The QueueReader to flush.
   * @param destination An iterator to the first position to flush the
   *        <i>queue<i> to.
   */
  template<typename Q, typename I> requires
    std::derived_from<
      dereference_t<Q>, QueueReader<typename dereference_t<Q>::Source>> &&
    std::output_iterator<I, typename dereference_t<Q>::Source>
  void flush(const Q& queue, I destination) {
    for_each(queue, [&] (auto&& value) {
      *destination = std::forward<decltype(value)>(value);
      ++destination;
    });
  }
}

#endif
