from functools import partial
import os
import sys
import threading
import unittest
import beam


class TestRoutines(unittest.TestCase):
  def test_concurrent_callbacks(self):
    interval = sys.getswitchinterval()
    sys.setswitchinterval(1)
    try:
      for iteration in range(10):
        with self.subTest(iteration=iteration):
          routines = beam.RoutineHandlerGroup()
          values = []
          count = 10 * (os.cpu_count() or 1)
          for i in range(count):
            routines.spawn(partial(values.append, i))
          routines.wait()
          self.assertEqual(sorted(values), list(range(count)))
    finally:
      sys.setswitchinterval(interval)

  def test_suspended_callbacks(self):
    count = os.cpu_count() or 1
    routines = beam.RoutineHandlerGroup()
    queues = [beam.Queue() for _ in range(2 * count)]
    started = [threading.Event() for _ in queues]
    finished = [threading.Event() for _ in queues]
    values = [None for _ in queues]

    def run(index):
      try:
        raise ValueError(index)
      except ValueError:
        started[index].set()
        value = queues[index].pop()
        values[index] = (value, sys.exception().args[0])
        finished[index].set()

    try:
      for wave in range(2):
        for i in range(wave * count, (wave + 1) * count):
          routines.spawn(partial(run, i))
        for i in range(wave * count, (wave + 1) * count):
          self.assertTrue(started[i].wait(10))
      for wave in range(2):
        for i in range(wave * count, (wave + 1) * count):
          queues[i].push(i)
        for i in range(wave * count, (wave + 1) * count):
          self.assertTrue(finished[i].wait(10))
      self.assertEqual(values, [(i, i) for i in range(2 * count)])
    finally:
      for i, queue in enumerate(queues):
        queue.push(i)
      routines.wait()
