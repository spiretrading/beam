import array
import gc
import unittest
import weakref
import beam


class Storage(bytearray):
    pass


class TestBufferView(unittest.TestCase):
    def test_borrowed_data(self):
        source = bytearray(b'a\x00bc')
        view = beam.BufferView(source, 3)
        self.assertEqual(view.size, 3)
        self.assertEqual(len(view), 3)
        self.assertEqual(view.get_data(), b'a\x00b')
        source[1] = ord('x')
        self.assertEqual(view.get_data(), b'axb')
        exported = memoryview(view)
        self.assertTrue(exported.readonly)
        self.assertEqual(exported.tobytes(), b'axb')
        with self.assertRaises(TypeError):
            exported[0] = 0

    def test_storage_lifetime(self):
        source = Storage(b'payload')
        reference = weakref.ref(source)
        view = beam.BufferView(source)
        nested = beam.BufferView(view)
        del source, view
        gc.collect()
        self.assertIsNotNone(reference())
        self.assertEqual(nested.get_data(), b'payload')
        del nested
        gc.collect()
        self.assertIsNone(reference())

    def test_resize(self):
        source = bytearray(b'payload')
        view = beam.BufferView(source)
        with self.assertRaises(BufferError):
            source.extend(b'x')
        exported = memoryview(view)
        del view
        gc.collect()
        with self.assertRaises(BufferError):
            source.extend(b'x')
        self.assertEqual(exported.tobytes(), b'payload')
        exported.release()
        del exported
        gc.collect()
        source.extend(b'x')
        self.assertEqual(source, b'payloadx')

    def test_empty(self):
        for source in (b'', bytearray(), memoryview(b'')):
            with self.subTest(source=source):
                view = beam.BufferView(source)
                self.assertFalse(view)
                self.assertTrue(view.is_empty())
                self.assertEqual(view.size, 0)
                self.assertEqual(view.get_data(), b'')
                self.assertEqual(memoryview(view).tobytes(), b'')
        self.assertEqual(beam.BufferView(b'payload', 0).get_data(), b'')

    def test_byte_count(self):
        source = array.array('I', [1, 2, 3])
        view = beam.BufferView(source)
        self.assertEqual(view.size, len(source) * source.itemsize)
        self.assertEqual(view.get_data(), source.tobytes())

    def test_invalid_range(self):
        with self.assertRaises(ValueError):
            beam.BufferView(b'abc', 4)
        with self.assertRaises(ValueError):
            beam.BufferView(memoryview(b'abcdef')[::2])


if __name__ == '__main__':
    unittest.main()
