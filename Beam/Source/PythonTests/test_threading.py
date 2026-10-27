from datetime import timedelta
import threading
import unittest
import beam


class TestThreading(unittest.TestCase):
    def test_condition_notification(self):
        for timed in (False, True):
            with self.subTest(timed=timed):
                mutex = beam.Mutex()
                condition = beam.ConditionVariable()
                started = threading.Event()
                values = []
                received = []

                def consume():
                    with mutex:
                        started.set()
                        while not values:
                            if timed:
                                if not condition.wait_for(mutex,
                                        timedelta(seconds=5)):
                                    return
                            else:
                                condition.wait(mutex)
                        received.append(values[0])

                consumer = threading.Thread(target=consume, daemon=True)
                consumer.start()
                self.assertTrue(started.wait(5))
                with mutex:
                    values.append(42)
                    condition.notify_one()
                consumer.join(5)
                self.assertFalse(consumer.is_alive())
                self.assertEqual(received, [42])

    def test_condition_timeout_owns_mutex(self):
        mutex = beam.Mutex()
        condition = beam.ConditionVariable()
        acquired = []

        def probe():
            locked = mutex.try_lock()
            acquired.append(locked)
            if locked:
                mutex.unlock()

        with mutex:
            self.assertFalse(condition.wait_for(mutex, timedelta(0)))
            contender = threading.Thread(target=probe, daemon=True)
            contender.start()
            contender.join(5)
            self.assertFalse(contender.is_alive())
            self.assertEqual(acquired, [False])

    def test_recursive_mutex_context(self):
        mutex = beam.RecursiveMutex()
        with mutex:
            with mutex:
                self.assertTrue(mutex.try_lock())
                mutex.unlock()
        self.assertTrue(mutex.try_lock())
        mutex.unlock()
