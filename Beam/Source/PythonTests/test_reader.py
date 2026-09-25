from concurrent.futures import Future
import ctypes
import gc
import unittest
import beam


class TestReader(unittest.TestCase):
    def setUp(self):
        self.operations = beam.Queue()
        self.reader = beam.tests.TestReader(self.operations)
        self.routines = []
        self.addCleanup(self.close)

    def close(self):
        self.reader.close()
        self.operations.close()
        for routine in self.routines:
            routine.wait()

    def start(self, function, *args):
        result = Future()

        def run():
            try:
                result.set_result(function(*args))
            except Exception as e:
                result.set_exception(e)

        self.routines.append(beam.RoutineHandler(beam.spawn(run)))
        return result

    def test_read(self):
        for arguments, size in (((), ctypes.c_size_t(-1).value), ((3,), 3)):
            with self.subTest(size=size):
                buffer = beam.SharedBuffer(b'prefix')
                result = self.start(self.reader.read, buffer, *arguments)
                operation = self.operations.pop()
                self.assertIsInstance(operation,
                    beam.tests.TestReader.ReadOperation)
                self.assertEqual(operation.size, size)
                self.assertFalse(result.done())
                operation.result.set(beam.SharedBuffer(b'abc'))
                self.assertEqual(result.result(timeout=5), 3)
                self.assertEqual(buffer.get_data(), b'prefixabc')

    def test_poll(self):
        for available in (False, True):
            with self.subTest(available=available):
                result = self.start(self.reader.poll)
                operation = self.operations.pop()
                self.assertIsInstance(operation,
                    beam.tests.TestReader.PollOperation)
                operation.result.set(available)
                self.assertEqual(result.result(timeout=5), available)

    def test_result_exception(self):
        result = self.start(self.reader.read, beam.SharedBuffer())
        operation = self.operations.pop()
        with self.assertRaises(TypeError):
            operation.result.set_exception('invalid')
        self.assertFalse(result.done())
        service_result = operation.result
        del operation
        gc.collect()
        service_result.set_exception(ValueError('read failed'))
        with self.assertRaises(ValueError):
            result.result(timeout=5)

    def test_close(self):
        result = self.start(self.reader.read, beam.SharedBuffer())
        operation = self.operations.pop()
        self.reader.close()
        with self.assertRaises(beam.EndOfFileException):
            result.result(timeout=5)
        operation.result.set(beam.SharedBuffer(b'ignored'))
        with self.assertRaises(beam.EndOfFileException):
            self.reader.read(beam.SharedBuffer())
        self.assertIsNone(self.operations.try_pop())

    def test_reader_conversion(self):
        reader = beam.Reader(self.reader)
        buffer = beam.SharedBuffer()
        result = self.start(reader.read, buffer, 4)
        operation = self.operations.pop()
        self.assertEqual(operation.size, 4)
        operation.result.set(beam.SharedBuffer(b'data'))
        self.assertEqual(result.result(timeout=5), 4)
        self.assertEqual(buffer.get_data(), b'data')


if __name__ == '__main__':
    unittest.main()
